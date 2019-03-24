#pragma once

/*
用于表示对象是可复制的。其派生类必须是值语义的
*/

class copyable {
protected:
	copyable() = default;
	~copyable() = default;
};
