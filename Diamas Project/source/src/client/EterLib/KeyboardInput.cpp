#include "stdafx.h"
#include "KeyboardInput.hpp"

KeyboardInput::KeyboardInput()
{
    // ctor
}

void KeyboardInput::OnKeyDown(KeyCode code)
{
    m_keys.set(code);
}

void KeyboardInput::OnKeyUp(KeyCode code)
{
    m_keys.reset(code);
}

bool KeyboardInput::IsKeyPressed(KeyCode code) const
{
    return m_keys.test(code);
}
