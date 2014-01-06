/*
Copyright (c) 2013 Aerys

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include "minko/Common.hpp"

#include "minko/component/LuaScriptManager.hpp"

namespace minko
{
	class LuaContext
	{
		friend class component::LuaScriptManager;

	private:
		static int								_argc;
		static char**							_argv;
		static std::shared_ptr<scene::Node>		_root;
		static std::shared_ptr<AbstractCanvas>	_canvas;

	public:
		static
		void
		initialize(int argc, char** argv, std::shared_ptr<scene::Node> root, std::shared_ptr<AbstractCanvas> canvas);

	private:
		static
		std::shared_ptr<AbstractCanvas>
		getCanvas();

		static
		std::shared_ptr<component::SceneManager>
		getSceneManager();

		static
		std::shared_ptr<input::Mouse>
		getMouse();

		static
		std::shared_ptr<input::Keyboard>
		getKeyboard();

		static
		std::shared_ptr<input::Joystick>
		getJoystick(int id);

		static
		bool
		getOption(const std::string& optionName);
	};
}
