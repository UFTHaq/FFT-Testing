// Online C++ compiler to run C++ program online
#include <iostream>
#include <cmath>

int main() {
    // Write C++ code here
    std::cout << "Try programiz.pro\n\n";
    
    int size = 29999;
    std::cout << "size : " << size << std::endl;
    int N{1000};
    
    double coef = (double)size / N;
    std::cout << "real coef : " << coef << std::endl;
    int interval_a = (int)coef;
    std::cout << "int coef : " << interval_a << std::endl;
    int interval_b = interval_a + 1;
    std::cout << "int coef++ : " << interval_b << std::endl;
    
    double first_f = (double)((int)(coef * 10) % 10) / 10;
    std::cout << "first : " << first_f << std::endl;
    
    double sec_f = (double)((int)(coef * 100) % 10) / 100;
    std::cout << "second : " << sec_f << std::endl;
    
    double third_f = (double)((int)(coef * 1000) % 10) / 1000;
    std::cout << "third : " << third_f << std::endl;
    
    double x1 = 1.0 - first_f;
    double x2 = first_f;
    double y = sec_f;
    double z = third_f;
    
    double a = x1 - y - z;
    std::cout << "a : " << a << std::endl;
    double b = x2 + y + z;
    std::cout << "b : " << b << std::endl << std::endl;
    
    // double a = x1 - y;
    // std::cout << "a : " << a << std::endl;
    // double b = x2 + y;
    // std::cout << "b : " << b << std::endl << std::endl;
    int coefa = std::round(N * 10 * a) / 10;
    int coefb = std::round(N * 10 * b) / 10;
    std::cout << N << " x " << a << " = " << coefa << std::endl;
    std::cout << N << " x " << b << " = " << coefb << std::endl;

    int coef_a_x = coefa * interval_a;
    int coef_b_x = coefb * interval_b;
    
    std::cout << "coef_a_x -> " << interval_a << " x " << coef_a_x / interval_a << " = " << coef_a_x << std::endl;
    std::cout << "coef_b_x -> " << interval_b << " x " << coef_b_x / interval_b << " = " << coef_b_x << std::endl;
    
    int used = coef_a_x + coef_b_x;
    std::cout << "used : " << used << std::endl;
    
    int remain = size - used;
    std::cout << "remain : " << remain << std::endl;
    std::cout << std::endl;
    
    double new_coef{};
    if (coef_a_x > coef_b_x) new_coef = (double)coef_a_x / coef_b_x;
    else new_coef = (double)coef_b_x / coef_a_x;
    
    std::cout << "new coef : " << new_coef << std::endl;
    std::cout << std::endl;
    
    double pengali_a = ((double)coef_a_x / interval_a);
    double pengali_b = ((double)coef_b_x / interval_b);
    
    double new_coef2{};
    int high_intense_interval{};
    int low_intense_interval{};
    if (coef_a_x > coef_b_x) {
        new_coef2 = pengali_a / pengali_b;
        high_intense_interval = interval_b;
        low_intense_interval = interval_a;
    }
    else {
        new_coef2 = pengali_b / pengali_a;
        high_intense_interval = interval_a;
        low_intense_interval = interval_b;
    }

    double lower_pengali{};
    double higher_pengali{};
    if (pengali_a > pengali_b) {
        higher_pengali = pengali_a;
        lower_pengali = pengali_b;
    } else {
        lower_pengali = pengali_a;
        higher_pengali = pengali_b;
    }
    std::cout << "new coef2 : " << new_coef2 << std::endl;
    int seling = int(new_coef2);
    std::cout << "selang-seling tiap : " << seling << "x" << std::endl;
    
    int hasil_lower_and_new_coef2 = lower_pengali * seling;
    std::cout << "taktik : " << lower_pengali << " x " << seling << " = " << hasil_lower_and_new_coef2 << std::endl;
    int sisa = higher_pengali - hasil_lower_and_new_coef2;
    std::cout << "hasil : " << higher_pengali << " - " << hasil_lower_and_new_coef2 << " = " << sisa << std::endl;
    std::cout << "sisa " << sisa << " kali untuk " << low_intense_interval << std::endl;
    
    std::cout << std::endl;
    
    std::cout << "Formula Interval Sampling: \n";
    std::cout << "Step index : " << low_intense_interval << " x 1 -> " << high_intense_interval << " x " << seling << std::endl;
    
    
    
    return 0;
    
}
