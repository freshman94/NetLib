#pragma once

#include <string.h>
#include <iosfwd>	//iostream的前向声明

#include <base/Types.h>

/*
(const)string/char * ======> const char*
*/
class StringArg {
public:
	StringArg(const char* str)
		: str_(str) {}
	StringArg(const string& str)
		: str_(str.c_str()) {}
	const char* c_str() const { return str_; }
private:
	const char* str_;
};

class StringPiece {
public:
	//ctor: "const (unsigned) char*" or "string"
	StringPiece()
		:ptr_(NULL), length_(0) {}
	StringPiece(const char* str)
		:ptr_(str), length_(static_cast<int>(strlen(ptr_))) {}
	StringPiece(const unsigned char* str)
		:ptr_(reinterpret_cast<const char*>(str)),
		length_(static_cast<int>(strlen(ptr_))) {}
	StringPiece(const string& str)
		:ptr_(str.data()), length_(static_cast<int>(str.length())) {}
	StringPiece(const char* str, int len)
		:ptr_(str), length_(len) {}

	const char* data() const { return ptr_; }
	int size() const { return length_; }
	bool empty() const { return length_ == 0; }
	const char* begin() const { return ptr_; }
	const char* end() const { return ptr_ + length_; }

	void clear() { ptr_ = NULL; length_ = 0; }
	void set(const char* buffer, int len) { ptr_ = buffer; length_ = len; }
	void set(const char* str) {
		ptr_ = str;
		length_ = static_cast<int>(strlen(str));
	}
	void set(const void* buffer, int len) {
		ptr_ = reinterpret_cast<const char*>(buffer);
		length_ = len;
	}

	char operator[](int i) const { return ptr_[i]; }

	void remove_prefix(int n) {
		ptr_ += n;
		length_ -= n;
	}

	void remove_suffix(int n) {
		length_ -= n;
	}

	bool operator==(const StringPiece& x) const {
		return ((length_ == x.length_) &&
			(memcmp(ptr_, x.ptr_, length_) == 0));
	}

	bool operator!=(const StringPiece& x) const {
		return !(*this == x);
	}


	//定义StringPiece的二元谓词。使用宏定义多个二元谓词
#define STRINGPIECE_BINARY_PREDICATE(cmp,auxcmp)										\
	bool operator cmp (const StringPiece& s) const {									\
		int res = memcmp(ptr_, s.ptr_, length_ < s.length_ ? length_ : s.length_);		\
		return ((res auxcmp 0) || (res == 0 && (length_ cmp s.length_)));				\
	}

	STRINGPIECE_BINARY_PREDICATE(< , < );
	STRINGPIECE_BINARY_PREDICATE(<= , < );
	STRINGPIECE_BINARY_PREDICATE(> , > );
	STRINGPIECE_BINARY_PREDICATE(>= , > );
#undef STRINGPIECE_BINARY_PREDICATE

	int compare(const StringPiece& s) const {
		int r = memcmp(ptr_, s.ptr_, length_ < s.length_ ? length_ : s.length_);
		if (r == 0) {
			if (length_ < s.length_) r = -1;
			else if (length_ > s.length_) r = +1;
		}
		return r;
	}
	//返回相应的string对象
	string as_string() const {
		return string(data(), size());
	}

	void CopyToString(string* target) const {
		target->assign(ptr_, length_);
	}
	//x是否是this的前缀
	bool starts_with(const StringPiece& x) const {
		return ((length_ >= x.length_) && (memcmp(ptr_, x.ptr_, x.length_) == 0));
	}

private:
	const char* ptr_;
	int length_;

};

std::ostream& operator<<(std::ostream& o, const StringArg& piece);

