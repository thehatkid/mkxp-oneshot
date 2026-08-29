#ifndef ONESHOT_H
#define ONESHOT_H

#include <cstdint>
#include <string>
#include <vector>

struct OneshotPrivate;
struct RGSSThreadData;

class Oneshot
{
public:
	Oneshot(RGSSThreadData &threadData);
	~Oneshot();

	enum
	{
		MSG_INFO,
		MSG_YESNO,
		MSG_WARN,
		MSG_ERR
	};

	const std::string &os() const;
	const std::string &lang() const;
	const std::string &userName() const;
	const std::string &savePath() const;
	const std::string &docsPath() const;
	const std::string &gamePath() const;
	const std::string &journal() const;

	bool allowExit() const;
	bool exiting() const;

	bool obscuredDirty;
	bool obscuredCleared() const;
	const std::vector<uint8_t> &obscuredMap() const;

#ifdef __linux__
	std::string desktopEnv;
#endif

	void update();

	void setYesNo(const char *yes, const char *no);
	void setWindowPos(int x, int y);
	void setAllowExit(bool allowExit);
	void setExiting(bool exiting);
	void resetObscured();

	bool msgbox(int type, const char *body, const char *title);
	std::string textinput(const char* prompt, int char_limit, const char* fontName);

private:
	OneshotPrivate *p;
	RGSSThreadData &threadData;
};

#endif // ONESHOT_H
