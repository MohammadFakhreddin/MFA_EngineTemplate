#pragma once

#define MFA_VARIABLE1(variable, type, default)           \
protected:                                                      \
type _##variable = default;                                     \
public:                                                         \
void Set##variable(type value)                                  \
{                                                               \
    if (_##variable == value)                                   \
    {                                                           \
        return;                                                 \
    }                                                           \
    _##variable = value;                                        \
}                                                               \
[[nodiscard]]                                                   \
type Get##variable() const                                      \
{                                                               \
    return _##variable;                                         \
}                                                               \
protected:                                                      \

#define MFA_VARIABLE2(variable, type, default, onChange) \
protected:                                                      \
type _##variable = default;                                     \
public:                                                         \
void Set##variable(type value)                                  \
{                                                               \
    if (_##variable == value)                                   \
    {                                                           \
        return;                                                 \
    }                                                           \
    _##variable = value;                                        \
    onChange();                                                 \
}                                                               \
[[nodiscard]]                                                   \
type Get##variable() const                                      \
{                                                               \
    return _##variable;                                         \
}                                                               \
protected:                                                      \

#define MFA_UNIQUE_NAME(base_) MFA_CONCAT(base_, __COUNTER__)

#define MFA_CONCAT__IMPL(x_, y_) x_ ## y_
#define MFA_CONCAT(x_, y_) MFA_CONCAT__IMPL(x_, y_)
