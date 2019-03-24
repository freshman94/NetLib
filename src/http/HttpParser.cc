#include <net/Buffer.h>
#include <http/HttpParser.h>

//����������
bool HttpParser::parseRequestLine(const char* begin, const char* end) {
	bool succeed = false;
	const char* start = begin;
	const char* space = std::find(start, end, ' ');
	const char* question;
	const char* ampersand;
	const char* equal;

	while (requestLineState_ != ParseFinished) {
		switch (requestLineState_)
		{
		case ParseMethod:	//����method
			if (space != end && request_.setMethod(start, space)) {
				requestLineState_ = ParseURL;
				break;
			}
			else
				return false;
		case ParseURL:	//����·���Ͳ���
			start = space + 1;
			space = std::find(start, end, ' ');
			if (space != end) {
				question = std::find(start, space, '?');
				if (question != end) {
					request_.setPath(start, question);

					start = question + 1;
					//��ȡ�������Ƽ�ֵ
					while ((ampersand = std::find(start, space, '&')) != space) {
						equal = std::find(start, ampersand, '=');
						if (equal != ampersand) {
							request_.addParas(start, equal, ampersand);
						}
						start = ampersand + 1;
					}
					//�������һ������
					if (start != space) {
						equal = std::find(start, space, '=');
						if (equal != space) {
							request_.addParas(start, equal, space);
						}
					}
				}
				else {
					request_.setPath(start, space);
				}
				requestLineState_ = ParseVersion;
				break;
			}
			else
				return false;
		case ParseVersion:	//����Http�汾
			start = space + 1;
			succeed = end - start == 8 && std::equal(start, end - 1, "HTTP/1.");
			if (succeed) {
				if (*(end - 1) == '1') {
					request_.setVersion(HttpRequest::Http11);
				}
				else if (*(end - 1) == '0') {
					request_.setVersion(HttpRequest::Http10);
				}
				else {
					return false;
				}
				requestLineState_ = ParseFinished;
				break;
			}
		case ParseFinished:
			break;
		default:
			return false;
		}
	}
	return true;
}

//������������
bool HttpParser::parseRequest(Buffer* buf){
	bool ok = true;
	bool hasMore = true;
	const char *crlf, *colon, *start, *end, *ampersand, *equal;

	while (hasMore) {
		switch (state_)
		{
		case ParseRequestLine:	//����������
			crlf = buf->findCRLF();
			if (crlf) {
				ok = parseRequestLine(buf->readablePtr(), crlf);
				if (ok) {
					buf->retrieveUntil(crlf + 2);
					state_ = ParseHeaders;
				}
				else {
					hasMore = false;
				}
			}
			else {
				hasMore = false;
			}	
			break;
		case ParseHeaders:	//����ͷ��
			crlf = buf->findCRLF();
			if (crlf) {
				colon = std::find(buf->readablePtr(), crlf, ':');
				if (colon != crlf) {
					request_.addHeader(buf->readablePtr(), colon, crlf);
				}
				else {
					if (request_.getHeader("Content-Length").size() == 0) {
						state_ = ParseGotAll;
						hasMore = false;
					}
					else {
						bodyLength_ = stoi(request_.getHeader("Content-Length"));
						state_ = ParseBody;
					}
				}
				buf->retrieveUntil(crlf + 2);
			}
			else {
				hasMore = false;
			}
			break;
		case ParseBody:	//������Ϣ����
			if (buf->readableBytes() >= bodyLength_) {
				start = buf->readablePtr();
				end = buf->readablePtr() + bodyLength_;

				//���ʺϱ�����Ϊapplication/x-www-form-urlencoded
				while ((ampersand = std::find(start, end, '&')) != end) {
					equal = std::find(start, ampersand, '=');
					if (equal != ampersand)
						request_.addParas(start, equal, end);
					start = ampersand + 1;
				}
				if (start != end) {
					equal = std::find(start, ampersand, '=');
					if (equal != ampersand) {
						request_.addParas(start, equal, end);
					}
				}
				state_ = ParseGotAll;
				hasMore = false;
			}
			else {
				hasMore = false;
			}
			break;
		case ParseGotAll:	//�������
		default:
			hasMore = false;
			break;
		}
	}
	return ok;
}
