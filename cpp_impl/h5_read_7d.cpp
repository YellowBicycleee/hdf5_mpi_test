#include <H5Cpp.h>
#include <vector>
#include <iostream>
#include <array>
#include <string>
#include <stdexcept>
#include <complex>

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

int main() {
    try {
        const std::string filename = "test_7d.hdf5";
        const std::string dataset_name = "LatticeMatrix";

        // 打开HDF5文件
        H5::H5File file(filename, H5F_ACC_RDONLY);
        H5::DataSet dataset = file.openDataSet(dataset_name);
        
        // 获取数据空间
        H5::DataSpace dataspace = dataset.getSpace();
        
        // 获取维度信息
        const int ndims = dataspace.getSimpleExtentNdims();
        std::vector<hsize_t> dims(ndims);
        dataspace.getSimpleExtentDims(dims.data(), nullptr);
        
        // 验证维度
        if (ndims != 7) {
            throw std::runtime_error("数据集维度不是7维");
        }
        
        // 创建Array7D对象
        Array7D<std::complex<double>> array7d(dims[1], dims[2], dims[3], dims[4], dims[5]);
        
        // 读取数据
        dataset.read(reinterpret_cast<double*>(array7d.data_ptr()), H5::PredType::NATIVE_DOUBLE);
        
        // 验证数据（打印部分数据作为示例）
        std::cout << "读取的部分数据示例：\n";
        for (size_t dim = 0; dim < 1; ++dim) {
            for (size_t t = 0; t < 1; ++t) {
                for (size_t z = 0; z < 1; ++z) {
                    for (size_t y = 0; y < 1; ++y) {
                        for (size_t x = 0; x < 1; ++x) {
                            for (size_t c1 = 0; c1 < array7d.get_Nc(); ++c1) {
                                for (size_t c2 = 0; c2 < array7d.get_Nc(); ++c2) {
                                    std::cout << "数据[" << dim << "][" << t << "][" 
                                              << z << "][" << y << "][" << x << "]["
                                              << c1 << "][" << c2 << "] = "
                                              << array7d(dim, t, z, y, x, c1, c2)
                                              << "\n";
                                }
                            }
                        }
                    }
                }
            }
        }
        
        return 0;
    } catch (const H5::Exception& e) {
        std::cerr << "HDF5错误：" << e.getCDetailMsg() << '\n';
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "错误：" << e.what() << '\n';
        return 1;
    }
} 