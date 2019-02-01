typedef int main_func(int, char**);
main_func main_editor;

int main(int argc, char**argv) {
	return main_editor(argc, argv);
}