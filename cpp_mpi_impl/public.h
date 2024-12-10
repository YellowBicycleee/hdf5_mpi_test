#pragma once

enum QcuDims {
    X_DIM = 0,
    Y_DIM = 1,
    Z_DIM = 2,
    T_DIM = 3,
    Nd
};

template <typename T>
class Array7D {
private:
    constexpr static int Ndim = 4;  
    std::vector<T> data;
    size_t Lt, Lz, Ly, Lx, Nc;
    
    // 私有的索引计算方法
    size_t index(size_t dim, size_t t, size_t z, size_t y, 
                 size_t x, size_t c1, size_t c2) const {
        return ((((((dim * Lt + t) * Lz + z) * Ly + y) * Lx + x) * Nc + c1) * Nc + c2);
    }
    
public:
    Array7D(size_t t_size, size_t z_size, size_t y_size, 
            size_t x_size, size_t color_size) 
        : Lt(t_size), Lz(z_size), Ly(y_size), Lx(x_size), Nc(color_size) {
        data.resize(Ndim * Lt * Lz * Ly * Lx * Nc * Nc);
    }

    T& operator()(size_t dim, size_t t, size_t z, size_t y, 
                  size_t x, size_t c1, size_t c2) {
        return data[index(dim, t, z, y, x, c1, c2)];
    }

    const T& operator()(size_t dim, size_t t, size_t z, size_t y, 
                       size_t x, size_t c1, size_t c2) const {
        return data[index(dim, t, z, y, x, c1, c2)];
    }

    // 移到public部分的方法
    T* data_ptr() { return data.data(); }
    const T* data_ptr() const { return data.data(); }
    
    size_t get_Lt() const { return Lt; }
    size_t get_Lz() const { return Lz; }
    size_t get_Ly() const { return Ly; }
    size_t get_Lx() const { return Lx; }
    size_t get_Nc() const { return Nc; }
    static constexpr size_t get_Ndim() { return Ndim; }
};


struct Coords {
    int data[4];

    int X() const { return data[X_DIM]; }
    int Y() const { return data[Y_DIM]; }
    int Z() const { return data[Z_DIM]; }
    int T() const { return data[T_DIM]; }
};