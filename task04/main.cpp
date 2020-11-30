#include <iostream>
#include <windows.h>
#include <vector>
#include <ctime>
#include <omp.h>

bool endOfExam = false;    // Флаг остановки экзамена

/**
 * Функция реализующая поток студента
 * @param param параметр с данными о студенте
 * @return код завершения потока
 */
void Student(int param) {
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

#pragma omp critical
    std::cout << "Student " << studNumber << ": take a ticket and prepare his answer" << std::endl;
    Sleep(rand() % 3000 + 2000); //Студент готовится к ответу

    WaitForSingleObject(teacherReady, INFINITE); //Студент ждет готовости преподавателя принимать его ответ
#pragma omp critical
    std::cout << "Student " << studNumber << " start to answer." << std::endl;

    data[0] = studNumber; //Студент передает свой номер преподавателю
    SetEvent(dataReady); //Дает понять преподавателю, что готов отвечать

    WaitForSingleObject(serverAnswer, INFINITE); //Ожидает ответа от преподавателя
#pragma omp critical
    std::cout << "Student " << studNumber << ": have a mark " << data[1] << std::endl;
}

/**
 * Реализует преподавателя
 * @param param
 * @return
 */
void Teacher(int countStudents) {
    //События преподавателя для синхронизации
    HANDLE teacherReady = CreateEvent(nullptr, FALSE, FALSE, LPCSTR("teacherReady")); //Готовность учителя принимать ответ
    HANDLE dataReady = CreateEvent(nullptr, FALSE, FALSE, LPCSTR("dataReady")); //Переданы ли данные студентом в память
    HANDLE serverAnswer = CreateEvent(nullptr, FALSE, FALSE, LPCSTR("serverAnswer")); //Учитель дал ответ студенту

    //Создаем общую память для общения студентов и преподавателей
    HANDLE mapFile = CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, sizeof(int) * 2, LPCSTR("MyShared"));
    int *data = (int*) MapViewOfFile(mapFile, FILE_MAP_READ | FILE_MAP_WRITE,0, 0, 0);

    int checkedStudents = 0;
    //Пока экзамен не окончен учитель принимает ответы студентов
    while (checkedStudents < countStudents) {
        SetEvent(teacherReady); //Устанавливает готовность учителя принимать ответы
        WaitForSingleObject(dataReady, INFINITE); //Ожидает пока студент предаст свои данные в память
#pragma omp critical
        std::cout << "Teacher: start to receive answer from student " << data[0] << std::endl;
        Sleep(rand() % 2000 + 1000); //Принимается ответ студента

        data[1] = rand() % 11; //Определяется оценка
#pragma omp critical
        std::cout << "Teacher: set a mark " << data[1] << " to student " << data[0] << std::endl;

        SetEvent(serverAnswer); //Преподаватель дает понять студенту, что закончил проверку
        checkedStudents++;
    }
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
    }
    return number;
}

int main() {
    srand(time(0));

    int studentsCount = ReadNumber(1, 100, "count of students"); //Считываем количество студентов
    auto *students = new HANDLE[studentsCount];
#pragma omp parallel num_threads(studentsCount + 1)
    {
        if (omp_get_thread_num() > 0) {
            int i = omp_get_thread_num();
            Student(i);
        } else
            Teacher(studentsCount);
    }
    WaitForMultipleObjects(studentsCount, &students[0], TRUE, INFINITE);
    studentsCount = 0;
    endOfExam = true; //Заканчиваем экзамен
    delete[] students;
    return 0;
}
