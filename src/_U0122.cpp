#include <string>
#include <direct.h>
#include <Windows.h>
std::string open_file_browser(const std::string & path, bool save, const std::string& ext) {
	OPENFILENAMEA fn = { 0, };
	char buf[1024];
	fn.lStructSize = sizeof(fn);
	fn.lpstrFile = buf;
	fn.lpstrFile[0] = '\0';
	fn.nMaxFile = sizeof(buf) - 1;

	char filter[1024];
	if (ext.size() > 10) return "";
	sprintf(filter, "%s(*.%s)%c*.%s%cAll%c*.*%c", ext.c_str(), ext.c_str(), 0, ext.c_str(), 0, 0, 0);
	
	fn.lpstrFilter = filter;
	fn.nFilterIndex = 1;
	fn.nMaxFileTitle = 0;
	fn.lpstrFileTitle = NULL;
	fn.lpstrInitialDir = NULL;
	if (save) {
		//fn.Flags = OFN_OVERWRITEPROMPT;
		GetSaveFileNameA(&fn);
	}
	else {
		fn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
		GetOpenFileNameA(&fn);
	}
	return buf;
}
