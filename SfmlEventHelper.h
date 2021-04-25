#pragma once

#include <stdexcept>
#include <variant>
#include <SFML/Graphics.hpp>

namespace sf::Utils
{
    struct ClosedEvent{};
    struct ResizedEvent : Event::SizeEvent {};
    struct LostFocus{};
    struct GainedFocus{};
    struct TextEntered : Event::TextEvent {};
    struct KeyPressed: Event::KeyEvent {};
    struct KeyReleased: Event::KeyEvent {};
    struct MouseWheelMoved : Event::MouseWheelEvent {};
    struct MouseWheelScrolled: Event::MouseWheelScrollEvent {};
    struct MouseButtonPressed : Event::MouseButtonEvent {};
    struct MouseButtonReleased : Event::MouseButtonEvent {};
    struct MouseMoved : Event::MouseMoveEvent {};
    struct MouseEntered{};
    struct MouseLeft{};
    struct JoystickButtonPressed : Event::JoystickButtonEvent {};
    struct JoystickButtonReleased : Event::JoystickButtonEvent {};
    struct JoystickMoved : Event::JoystickMoveEvent {};
    struct JoystickConnected : Event::JoystickConnectEvent {};
    struct JoystickDisconnected : Event::JoystickConnectEvent {};
    struct TouchBegan : Event::TouchEvent{};
    struct TouchMoved : Event::TouchEvent{};
    struct TouchEnded : Event::TouchEvent{};
    struct SensorChanged : Event::SensorEvent {};

    using SfmlEvent =
        std::variant<ClosedEvent, ResizedEvent, LostFocus, GainedFocus, TextEntered, KeyPressed, KeyReleased, MouseWheelMoved, MouseWheelScrolled, MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseEntered, MouseLeft, JoystickButtonPressed, JoystickButtonReleased, JoystickMoved,
                     JoystickConnected, JoystickDisconnected, TouchBegan, TouchMoved, TouchEnded, SensorChanged

    >;

    SfmlEvent MakeTypeSafeEvent(Event event)
    {
        switch (event.type)
        {
            case Event::Closed: ///< The window requested to be closed (no data)
                return ClosedEvent{};
            case Event::Resized: ///< The window was resized (data in event.size)
                return ResizedEvent{event.size};
            case Event::LostFocus: ///< The window lost the focus (no data)
                return LostFocus{};
            case Event::GainedFocus: ///< The window gained the focus (no data)
                return GainedFocus{};
            case Event::TextEntered: ///< A character was entered (data in event.text)
                return TextEntered{event.text};
            case Event::KeyPressed: ///< A key was pressed (data in event.key)
                return KeyPressed{event.key};
            case Event::KeyReleased: ///< A key was released (data in event.key)
                return KeyReleased{event.key};
            case Event::MouseWheelMoved: ///< The mouse wheel was scrolled (data in event.mouseWheel) (deprecated)
                return MouseWheelMoved{event.mouseWheel}; 
            case Event::MouseWheelScrolled: ///< The mouse wheel was scrolled (data in event.mouseWheelScroll)
                return MouseWheelScrolled{event.mouseWheelScroll};
            case Event::MouseButtonPressed: ///< A mouse button was pressed (data in event.mouseButton)
                return MouseButtonPressed{event.mouseButton};
            case Event::MouseButtonReleased: ///< A mouse button was released (data in event.mouseButton)
                return MouseButtonReleased{event.mouseButton};
            case Event::MouseMoved: ///< The mouse cursor moved (data in event.mouseMove)
                return MouseMoved{event.mouseMove};
            case Event::MouseEntered: ///< The mouse cursor entered the area of the window (no data)
                return MouseEntered{};
            case Event::MouseLeft: ///< The mouse cursor left the area of the window (no data)
                return MouseLeft{};
            case Event::JoystickButtonPressed: ///< A joystick button was pressed (data in event.joystickButton)
                return JoystickButtonPressed{event.joystickButton};
            case Event::JoystickButtonReleased: ///< A joystick button was released (data in event.joystickButton)
                return JoystickButtonReleased{event.joystickButton};
            case Event::JoystickMoved: ///< The joystick moved along an axis (data in event.joystickMove)
                return JoystickMoved{event.joystickMove};
            case Event::JoystickConnected: ///< A joystick was connected (data in event.joystickConnect)
                return JoystickConnected{event.joystickConnect};
            case Event::JoystickDisconnected:  ///< A joystick was disconnected (data in event.joystickConnect)
                return JoystickDisconnected{event.joystickConnect};
            case Event::TouchBegan: ///< A touch event began (data in event.touch)
                return TouchBegan{event.touch};
            case Event::TouchMoved: ///< A touch moved (data in event.touch)
                return TouchMoved{event.touch};
            case Event::TouchEnded: ///< A touch event ended (data in event.touch)
                return TouchEnded{event.touch};
            case Event::SensorChanged: ///< A sensor value changed (data in event.sensor)
                return SensorChanged{event.sensor};
            case Event::Count:
                break;
        }

        throw std::logic_error{"unsupported SFML event"};
    }
}