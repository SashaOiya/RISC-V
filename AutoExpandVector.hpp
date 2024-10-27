#include <iostream>
#include <vector>

template <typename data_t>
class AutoExpandVector {
private:
    std::vector<data_t> data;

public:
    AutoExpandVector() {}

    // Доступ к элементу с автоматическим расширением
    T& operator[](size_t index) {
        if (index >= data.size()) {
            data.resize(index + 1);  // Автоматическое расширение до нужного индекса
        }
        return data[index];
    }

    size_t size() const {
        return data.size();
    }

    void push_back(const T& value) {
        data.push_back(value);
    }

    // Пример метода, чтобы просто вывести данные вектора
    void print() const {
        for (const auto& item : data) {
            std::cout << item << " ";
        }
        std::cout << std::endl;
    }
};
