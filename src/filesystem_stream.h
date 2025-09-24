/*
 * This file is part of EasyRPG Player.
 *
 * EasyRPG Player is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * EasyRPG Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EP_FILESYSTEM_STREAM_H
#define EP_FILESYSTEM_STREAM_H

// Headers
#include <cassert>
#include <istream>
#include <ostream>
#include "filesystem.h"
#include "utils.h"
#include "system.h"
#include "PlayerCompat/off_t.h"
#include "PlayerCompat/ssize_t.h"

namespace Filesystem_Stream {
	class InputStream final : public std::istream {
	public:
		explicit InputStream(): std::istream(nullptr) {}
		explicit InputStream(std::streambuf* sb, std::string name);
		~InputStream() override;
		InputStream(const InputStream&) = delete;
		InputStream& operator=(const InputStream&) = delete;
		InputStream(InputStream&& is) noexcept;
		InputStream& operator=(InputStream&& is) noexcept;

		std::string_view GetName() const;
		std::streampos GetSize() const;
		std::streampos GetPosition() const;
		void Close();

		template <typename T>
		bool ReadIntoObj(T& obj);

	private:
		template <typename T>
		bool Read0(T& obj);

		std::string name;
		mutable bool size_cached = false;
		mutable std::streampos size = 0;
	};

	class OutputStream final : public std::ostream {
	public:
		explicit OutputStream(): std::ostream(nullptr) {}
		explicit OutputStream(std::streambuf* sb, FilesystemView fs, std::string name);
		~OutputStream() override;
		OutputStream(const OutputStream&) = delete;
		OutputStream& operator=(const OutputStream&) = delete;
		OutputStream(OutputStream&& os) noexcept;
		OutputStream& operator=(OutputStream&& os) noexcept;

		std::string_view GetName() const;
		void Close();

	private:
		FilesystemView fs;
		std::string name;
	};

	/** Streambuf interface for an in-memory buffer. Does not take ownership of the buffer. */
	class InputMemoryStreamBufView : public std::streambuf {
	public:
		explicit InputMemoryStreamBufView(Span<uint8_t> buffer_view);
		InputMemoryStreamBufView(InputMemoryStreamBufView const& other) = delete;
		InputMemoryStreamBufView const& operator=(InputMemoryStreamBufView const& other) = delete;

	protected:
		std::streambuf::pos_type seekoff(std::streambuf::off_type offset, std::ios_base::seekdir dir, std::ios_base::openmode mode) override;
		std::streambuf::pos_type seekpos(std::streambuf::pos_type pos, std::ios_base::openmode mode) override;

	private:
		Span<uint8_t> buffer_view;
	};

	/** Streambuf interface for an in-memory buffer. Takes ownership of the buffer. */
	class InputMemoryStreamBuf : public InputMemoryStreamBufView {
	public:
		explicit InputMemoryStreamBuf(std::vector<uint8_t> buffer);
		InputMemoryStreamBuf(InputMemoryStreamBuf const& other) = delete;
		InputMemoryStreamBuf const& operator=(InputMemoryStreamBuf const& other) = delete;

	private:
		std::vector<uint8_t> buffer;
	};

#ifdef USE_CUSTOM_FILEBUF
	class FdStreamBuf : public std::streambuf {
	public:
		FdStreamBuf(int fd, bool is_read);
		FdStreamBuf(FdStreamBuf const& other) = delete;
		FdStreamBuf const& operator=(FdStreamBuf const& other) = delete;
		~FdStreamBuf();

	protected:
		// Reading
		int_type underflow() override;
		std::streambuf::pos_type seekoff(std::streambuf::off_type offset, std::ios_base::seekdir dir, std::ios_base::openmode mode) override;
		std::streambuf::pos_type seekpos(std::streambuf::pos_type pos, std::ios_base::openmode) override;

		// Writing
		int_type overflow(int c = EOF) override;
		int sync() override;
	private:
		// Reading
		void clear_buffer();
		ssize_t bytes_remaining() const;
		off_t file_offset = 0;

		// Both
		int fd;
		bool is_read; // Streams can be read and write but we only always use one mode
		std::array<char, USE_CUSTOM_FILEBUF> buffer;
	};
#endif

	static constexpr std::ios_base::seekdir CSeekdirToCppSeekdir(int origin);

	static constexpr int CppSeekdirToCSeekdir(std::ios_base::seekdir origin);
};

template<typename T>
inline bool Filesystem_Stream::InputStream::Read0(T& obj) {
	return read(reinterpret_cast<char*>(&obj), sizeof(obj)).gcount() == sizeof(obj);
}

template<typename T>
inline bool Filesystem_Stream::InputStream::ReadIntoObj(T& obj) {
	return Read0(obj);
}

template <>
inline bool Filesystem_Stream::InputStream::ReadIntoObj(int16_t& obj) {
	bool success = Read0(obj);
	uint16_t uobj = obj;
	Utils::SwapByteOrder(uobj);
	obj = uobj;
	return success;
}

template <>
inline bool Filesystem_Stream::InputStream::ReadIntoObj(uint16_t& obj) {
	bool success = Read0(obj);
	Utils::SwapByteOrder(obj);
	return success;
}

template <>
inline bool Filesystem_Stream::InputStream::ReadIntoObj(int32_t& obj) {
	bool success = Read0(obj);
	uint32_t uobj = obj;
	Utils::SwapByteOrder(uobj);
	obj = uobj;
	return success;
}

template <>
inline bool Filesystem_Stream::InputStream::ReadIntoObj(uint32_t& obj) {
	bool success = Read0(obj);
	Utils::SwapByteOrder(obj);
	return success;
}

constexpr std::ios_base::seekdir Filesystem_Stream::CSeekdirToCppSeekdir(int origin) {
	switch (origin) {
		case SEEK_SET:
			return std::ios_base::beg;
		case SEEK_CUR:
			return std::ios_base::cur;
		case SEEK_END:
			return std::ios_base::end;
		default:
			assert(false);
			return std::ios_base::beg;
	}
}

constexpr int Filesystem_Stream::CppSeekdirToCSeekdir(std::ios_base::seekdir origin) {
	switch (origin) {
		case std::ios_base::beg:
			return SEEK_SET;
		case std::ios_base::cur:
			return SEEK_CUR;
		case std::ios_base::end:
			return SEEK_END;
		default:
			assert(false);
			return SEEK_SET;
	}
}

#endif
