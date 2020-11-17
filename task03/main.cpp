#include <iostream>
#include <windows.h>
#include <vector>
#include <ctime>

bool endOfExam = false;    // Флаг остановки экзамена

/**
 * Функция реализующая поток студента
 * @param param параметр с данными о студенте
 * @return код завершения потока
 */
DWORD WINAPI Student(PVOID param) {
    HANDLE teacherReady, dataReady, serverAnswer;

    //Открываем события до тех пор, пока все из них не будут проинициализированы
    while (teacherReady == nullptr || dataReady == nullptr || serverAnswer == nullptr)
    {
        teacherReady = OpenEvent(EVENT_ALL_ACCESS, FALSE, LPCSTR("teacherReady")); //Готовность учителя принимать ответ
        dataReady = OpenEvent(EVENT_ALL_ACCESS, FALSE, LPCSTR("dataReady")); //Переданы ли данные студентом в память
        serverAnswer = OpenEvent(EVENT_ALL_ACCESS, FALSE, LPCSTR("serverAnswer")); //Учитель дал ответ студенту
        Sleep(10);
    }

    //Создаем общую память для общения студентов и преподавателя
    HANDLE mapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, LPCSTR("MyShared"));
    while (mapFile == nullptr) {
        Sleep(10);
        mapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, LPCSTR("MyShared"));
    }
    int *data = (int*)MapViewOfFile(mapFile, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);

    int studNumber = (DWORD) param; //получаем параметр переданный в поток
    srand(time(0));

    std::cout << "Student " << studNumber << ": take a ticket and prepare his answer" << std::endl;
    Sleep(rand() % 3000 + 2000); //Студент готовится к ответу

    WaitForSingleObject(teacherReady, INFINITE); //Студент ждет готовости преподавателя принимать его ответ
    std::cout << "Student " << studNumber << " start to answer." << std::endl;

    data[0] = studNumber; //Студент передает свой номер преподавателю
    SetEvent(dataReady); //Дает понять преподавателю, что готов отвечать

    WaitForSingleObject(serverAnswer, INFINITE); //Ожидает ответа от преподавателя
    std::cout << "Student " << studNumber << ": have a mark " << data[1] << std::endl;
    return 0;
}

/**
 * Реализует преподавателя
 * @param param
 * @return
 */
DWORD WINAPI Teacher(PVOID param) {
    //События преподавателя для синхронизации
    HANDLE teacherReady = CreateEvent(nullptr, FALSE, FALSE, LPCSTR("teacherReady")); //Готовность учителя принимать ответ
    HANDLE dataReady = CreateEvent(nullptr, FALSE, FALSE, LPCSTR("dataReady")); //Переданы ли данные студентом в память
    HANDLE serverAnswer = CreateEvent(nullptr, FALSE, FALSE, LPCSTR("serverAnswer")); //Учитель дал ответ студенту

    //Создаем общую память для общения студентов и преподавателей
    HANDLE mapFile = CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, sizeof(int) * 2, LPCSTR("MyShared"));
    int *data = (int*) MapViewOfFile(mapFile, FILE_MAP_READ | FILE_MAP_WRITE,0, 0, 0);

    //Пока экзамен не окончен учитель принимает ответы студентов
    while (!endOfExam) {
        SetEvent(teacherReady); //Устанавливает готовность учителя принимать ответы
        WaitForSingleObject(dataReady, INFINITE); //Ожидает пока студент предаст свои данные в память

        std::cout << "Teacher: start to receive answer from student " << data[0] << std::endl;
        Sleep(rand() % 2000 + 1000); //Принимается ответ студента

        data[1] = rand() % 11; //Определяется оценка
        std::cout << "Teacher: set a mark " << data[1] << " to student " << data[0] << std::endl;

        SetEvent(serverAnswer); //Преподаватель дает понять студенту, что закончил проверку
    }
    return 0;
}

/**
 * Считывает число
 * @param minVal максимальное значение вводимого числа
 * @param maxVal минимальное значение вводимого числа
 * @param str название вводимых данных
 * @return считаное число
 */
int ReadNumber(int minVal, int maxVal, std::string str) {
    int number;
    std::cout << "Input count of students (" << minVal << ";" << maxVal << "):";
    std::cin >> number;
    while (number < minVal || number > maxVal) {
        std::cout << "Incorrect input..." << std::endl;
        std::cout << "Input " << str << " again:";
        std::cin >> number;
    };
}

/**
 * Создает массив потоков-студентов
 * @param studentsCount количество студентов
 * @param ticketsCount количество билетов
 * @param threadId
 * @return массив потоков-студентов
 */
HANDLE* CreateStudents(int studentsCount, DWORD threadId) {
    auto *students = new HANDLE[studentsCount];
    DWORD t;
    for (short i = 0; i < studentsCount; i++) {
        t = i + 1;
        students[i] = CreateThread(NULL, 0, Student, (PVOID) t, NULL, &threadId);
        Sleep(rand() % 700 + 300);
    }
    return students;
}

int main() {
    DWORD threadId;
    HANDLE serverThread = CreateThread(nullptr, 0, Teacher, nullptr, 0, &threadId);

    int studentsCount = ReadNumber(1, 100, "count of students"); //Считываем количество студентов
    auto *students = CreateStudents(studentsCount, threadId); //Создаем массив студентов
    WaitForMultipleObjects(studentsCount, &students[0], TRUE, INFINITE);
    studentsCount = 0;

    endOfExam = true; //Заканчиваем экзамен
    delete[] students;
    return 0;
}
