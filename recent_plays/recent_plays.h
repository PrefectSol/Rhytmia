// Приведенный ниже блок ifdef — это стандартный метод создания макросов, упрощающий процедуру
// экспорта из библиотек DLL. Все файлы данной DLL скомпилированы с использованием символа RECENTPLAYS_EXPORTS
// Символ, определенный в командной строке. Этот символ не должен быть определен в каком-либо проекте,
// использующем данную DLL. Благодаря этому любой другой проект, исходные файлы которого включают данный файл, видит
// функции RECENTPLAYS_API как импортированные из DLL, тогда как данная DLL видит символы,
// определяемые данным макросом, как экспортированные.
#ifdef RECENTPLAYS_EXPORTS
#define RECENTPLAYS_API __declspec(dllexport)
#else
#define RECENTPLAYS_API __declspec(dllimport)
#endif

// Этот класс экспортирован из библиотеки DLL
class RECENTPLAYS_API Crecentplays {
public:
	Crecentplays(void);
	// TODO: добавьте сюда свои методы.
};

extern RECENTPLAYS_API int nrecentplays;

RECENTPLAYS_API int fnrecentplays(void);
