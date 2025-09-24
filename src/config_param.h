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

#ifndef EP_CONFIG_PARAM_H
#define EP_CONFIG_PARAM_H

#include "string_view.h"
#include "filefinder.h"
#include <array>
#include <cstdint>
#include <lcf/inireader.h>
#include <limits>
#include <string>
#include <vector>
#include <algorithm>
#include <type_traits>
#include <lcf/flag_set.h>

namespace {
	inline std::string ParamValueToString(std::string_view s) {
		return ToString(s);
	}

	inline std::string ParamValueToString(int i) {
		return std::to_string(i);
	}

	inline std::string ParamValueToString(double d) {
		return std::to_string(d);
	}

	inline std::string ParamValueToString(bool b) {
		return b ? "[ON]" : "[OFF]";
	}
}

/**
 * Base class for all ConfigParams
 */
template <typename T>
class ConfigParamBase {
public:
	using value_type = T;

	/**
	 * @param name Name shown in the settings scene
	 * @param description Description shown in the help window of the settings scene
	 * @param value Initial value
	 */
	ConfigParamBase(std::string_view name, std::string_view description, std::string_view config_section, std::string_view config_key, T value) :
		_name(name), _description(description), _config_section(config_section), _config_key(config_key), _value(value) {}

	/** @return current assigned value */
	T Get() const { return _value; }

	/**
	 * Sets the value.
	 * Setting fails when the option is locked or invisible or the value is not
	 * in a supported range of the option.
	 *
	 * @param value Value to set to
	 * @return Whether the setting succeeded. When it fails the old value remains.
	 */
	bool Set(const T& value) {
		if (IsLocked()) {
			return false;
		}

		if (IsValid(value)) {
			_value = value;
			return true;
		}
		return false;
	}

	/**
	 * Check whether a value is acceptable for this setting.
	 * This function is used for internal purposes.
	 *
	 * @see Set
	 * @param value Value to set to
	 * @return Whether the value is okay
	 */
	bool IsValid(const T& value) const {
		if (!IsOptionVisible()) {
			return false;
		}

		if (IsLocked()) {
			return value == this->_value;
		}

		return vIsValid(value);
	}

	/** @return Whether the option is displayed and supported */
	bool IsOptionVisible() const {
		return _visible;
	}

	/**
	 * Sets the visibility of the option in the settings scene.
	 * Additionally this indicates wheter the setting is supported at all.
	 * When not visible all write operations to the setting will fail.
	 *
	 * @see Lock To indicate that the setting is supported but currently constant
	 * @param visible Turns visibility on/off
	 */
	void SetOptionVisible(bool visible) {
		_visible = visible;
	}

	/** @return Whether the option is currently locked and cannot be altered */
	bool IsLocked() const {
		return _locked;
	}

	/**
	 * Locks the option. A locked option cannot be altered.
	 * Use this when the option is supported but cannot be altered currently due
	 * to a dependency on another option.
	 *
	 * @param value Value to lock the option with
	 * @return Whether the locking succeeded. When it fails the old value remains.
	 */
	bool Lock(T value) {
		_locked = false;

		if (!Set(value)) {
			_locked = true;
			return false;
		}

		_locked = true;
		return true;
	}

	/**
	 * Locks or unlocks the option. The current value stays.
	 *
	 * @see Lock
	 * @param locked turn locking on/off
	 */
	void SetLocked(bool locked) {
		_locked = locked;
	}

	/** @return name displayed in the settings scene */
	std::string_view GetName() const {
		return _name;
	}

	/**
	 * @param name new name of the setting
	 */
	void SetName(std::string_view name) {
		_name = name;
	}

	/** @return help text displayed in the settings scene */
	std::string_view GetDescription() const {
		return _description;
	}

	/**
	 * @param description new description of the setting
	 */
	void SetDescription(std::string_view description) {
		_description = description;
	}

	/** @return human readable representation of the value for the settings scene */
	virtual std::string ValueToString() const = 0;

	virtual bool FromIni(const lcf::INIReader& ini) {
		if (ini.HasValue(_config_section, _config_key)) {
			if constexpr (std::is_same_v<T, std::string>) {
				Set(std::string(ini.GetString(_config_section, _config_key, "")));
				return true;
			} else if constexpr (std::is_same_v<T, int>) {
				Set(ini.GetInteger(_config_section, _config_key, T()));
				return true;
			} else if constexpr (std::is_same_v<T, bool>) {
				Set(ini.GetBoolean(_config_section, _config_key, T()));
				return true;
			}
		}
		return false;
	}

	template <typename U = T, typename std::enable_if<!std::is_enum<U>::value, int>::type = 0>
	void ToIni(std::ostream& ini) const {
		if (IsOptionVisible()) {
			ini << _config_key << '=' << Get() << "\n";
		}
	}

protected:
	virtual bool vIsValid(const T& value) const = 0;

	std::string_view _name;
	std::string_view _description;
	std::string_view _config_section;
	std::string_view _config_key;
	T _value = {};

private:
	bool _visible = true;
	bool _locked = false;
};

/** A configuration parameter with no restrictions */
template <typename T>
class ConfigParam : public ConfigParamBase<T> {
public:
	explicit ConfigParam(std::string_view name, std::string_view description, std::string_view config_section, std::string_view config_key, T value = {}) :
		ConfigParamBase<T>(name, description, config_section, config_key, std::move(value)) {}

	bool vIsValid(const T&) const override {
		return true;
	}

	std::string ValueToString() const override {
		return ParamValueToString(this->Get());
	}
};

/** A configuration parameter which is locked by default */
template <typename T>
class LockedConfigParam final : public ConfigParam<T> {
public:
	explicit LockedConfigParam(std::string_view name, std::string_view description, T value = {}) :
		ConfigParam<T>(name, description, "", "", value) {
		this->Lock(value);
	}
};

using StringConfigParam = ConfigParam<std::string>;

/** A configuration parameter with a range */
template <typename T>
class RangeConfigParam : public ConfigParamBase<T> {
public:
	/** Construct with name and initial value */
	explicit RangeConfigParam(std::string_view name, std::string_view description, std::string_view config_section, std::string_view config_key, T value = {}) :
		ConfigParamBase<T>(name, description, config_section, config_key, std::move(value)) {}

	/** Construct with name and initial value, min, and max */
	RangeConfigParam(std::string_view name, std::string_view description, std::string_view config_section, std::string_view config_key, T value, T minval, T maxval) :
		ConfigParamBase<T>(name, description, config_section, config_key, std::move(value)) { SetRange(minval, maxval); }

	/**
	 * Check if a value is valid
	 * @param value the value to check
	 * @return true if this value can be set
	 */
	bool vIsValid(const T& value) const override {
		return value >= _min && value <= _max;
	}

	T GetMin() const {
		return _min;
	}

	T GetMax() const {
		return _max;
	}

	/**
	 * Set minimum allowed value.
	 * @param minval the new minimum
	 * @post If the current value is < minval, it will be set equal to minval
	 * @post If the current maximum < minval, this parameter is disabled.
	 */
	void SetMin(T minval) { SetRange(minval, _max); }

	/**
	 * Set maximum allowed value.
	 * @param maxval the new maximum
	 * @post If the current value is > maxval, it will be set equal to maxval
	 * @post If the current minimum > maxval, this parameter is disabled.
	 */
	void SetMax(T maxval) { SetRange(_min, maxval); }

	/**
	 * Set allowed range of values.
	 * @param minval the new maximum
	 * @param maxval the new maximum
	 * @post If the current value is outside the range, it will be clamped.
	 * @post If the minval > maxval, this parameter is disabled.
	 */
	void SetRange(T minval, T maxval) {
		this->SetOptionVisible(true);
		_min = minval;
		_max = maxval;
		this->_value = (this->_value < _min) ? _min : this->_value;
		this->_value = (this->_value > _max) ? _max : this->_value;
	}

	std::string ValueToString() const override {
		return ParamValueToString(this->_value);
	}

private:
	T _min = std::numeric_limits<T>::min();
	T _max = std::numeric_limits<T>::max();
};

/** A boolean configuration parameter */
class BoolConfigParam : public ConfigParamBase<bool> {
public:
	explicit BoolConfigParam(std::string_view name, std::string_view description, std::string_view config_section, std::string_view config_key, bool value)  :
		ConfigParamBase<bool>(name, description, config_section, config_key, value) {}

	bool vIsValid(const bool&) const override {
		return true;
	}

	bool Toggle() {
		if (Get()) {
			Set(false);
		} else {
			Set(true);
		}
		return Get();
	}

	std::string ValueToString() const override {
		return ParamValueToString(Get());
	}
};

template <typename E, size_t S>
class EnumConfigParam : public ConfigParamBase<E> {
public:
	EnumConfigParam(std::string_view name, std::string_view description, std::string_view config_section, std::string_view config_key, E value, std::array<std::string_view, S> values, std::array<std::string_view, S> tags, std::array<std::string_view, S> value_descriptions) :
		ConfigParamBase<E>(name, description, config_section, config_key, value), _values{ values }, _tags{ tags}, _value_descriptions{ value_descriptions } {
		for (size_t i = 0; i < S; ++i) {
			_valid[static_cast<E>(i)] = true;
		}
	}

	bool vIsValid(const E& value) const override {
		return _valid[value];
	}

	void ReplaceValidSet(lcf::FlagSet<E> valid) {
		_valid = std::move(valid);
		if (this->IsOptionVisible() && !this->IsValid(this->_value)) {
			this->_value = GetFirstValid();
		}
	}

	void AddToValidSet(E value) {
		_valid[value] = true;
	}

	void RemoveFromValidSet(const E& value) {
		_valid[value] = false;
		if (this->IsOptionVisible() && !this->IsValid(this->_value)) {
			this->_value = GetFirstValid();
		}
	}

	std::string ValueToString() const override {
		return ToString(_values[static_cast<int>(this->_value)]);
	}

	std::array<std::string_view, S> GetValues() const {
		return _values;
	}

	std::array<std::string_view, S> GetDescriptions() const {
		return _value_descriptions;
	}

	bool SetFromString(std::string_view value) {
		for (size_t i = 0; i < _tags.size(); ++i) {
			if (value == _tags[i]) {
				this->Set(static_cast<E>(i));
				return true;
			}
		}
		return false;
	}

	bool FromIni(const lcf::INIReader& ini) override {
		if (ini.HasValue(this->_config_section, this->_config_key)) {
			auto s = ini.GetString(this->_config_section, this->_config_key, "");
			for (size_t i = 0; i < _tags.size(); ++i) {
				if (s == _tags[i]) {
					this->Set(static_cast<E>(i));
					return true;
				}
			}
		}
		return false;
	}

	void ToIni(std::ostream& ini) const {
		if (this->IsOptionVisible()) {
			ini << this->_config_key << '=' << _tags[static_cast<int>(this->Get())] << "\n";
		}
	}

private:
	lcf::FlagSet<E> _valid;
	std::array<std::string_view, S> _values;
	std::array<std::string_view, S> _tags;
	std::array<std::string_view, S> _value_descriptions;
	bool _enabled = true;

	E GetFirstValid() const {
		for (size_t i = 0; i < _valid.size(); ++i) {
			auto e = static_cast<E>(i);
			if (_valid[e]) {
				return e;
			}
		}
		return E{};
	}
};

class PathConfigParam : public StringConfigParam {
public:
	PathConfigParam(std::string_view name, std::string_view description, std::string_view config_section, std::string_view config_key, std::string value) :
			StringConfigParam(name, description, config_section, config_key, value) {}

	std::string ValueToString() const override {
		return std::get<1>(FileFinder::GetPathAndFilename(Get()));
	}
};

#endif
