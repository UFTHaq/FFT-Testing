// Online C++ compiler to run C++ program online
#include <iostream>

int main() {
    // Write C++ code here
    std::cout << "Try programiz.pro\n\n";
    
    int size = 14320;
    std::cout << "size : " << size << std::endl;
    int N{1000};
    
    float coef = (float)size / N;
    std::cout << "real coef : " << coef << std::endl;
    int interval_a = (int)coef;
    std::cout << "int coef : " << interval_a << std::endl;
    int interval_b = interval_a + 1;
    std::cout << "int coef++ : " << interval_b << std::endl;
    
    float first_f = (float)((int)(coef * 10) % 10) / 10;
    std::cout << "first : " << first_f << std::endl;
    
    float sec_f = (float)((int)(coef * 100) % 10) / 100;
    std::cout << "second : " << sec_f << std::endl;
    
    float x1 = 1.0 - first_f;
    float x2 = first_f;
    float y = sec_f;
    
    float a = x1 - y;
    std::cout << "a : " << a << std::endl;
    float b = x2 + y;
    std::cout << "b : " << b << std::endl << std::endl;
    
    int coef_a_x = N * a * interval_a;
    std::cout << "coef_a_x -> " << interval_a << " : " << coef_a_x << std::endl;
    int coef_b_x = N * b * interval_b;
    std::cout << "coef_b_x -> " << interval_b << " : " << coef_b_x << std::endl;
    
    int used = coef_a_x + coef_b_x;
    std::cout << "used : " << used << std::endl;
    
    int remain = size - used;
    std::cout << "remain : " << remain << std::endl;

    return 0;
}
