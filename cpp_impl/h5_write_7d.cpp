#include <H5Cpp.h>
#include <vector>
#include <iostream>
#include <array>
#include <string>
#include <stdexcept>

template <typename T>
class Array7D {
private:
    constexpr static int Ndim = 4;  // 四维空间
    std::vector<T> data;
    size_t Lt, Lz, Ly, Lx, Nc;
    
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

    // 添加获取数据指针的方法
    T* data_ptr() { return data.data(); }
    const T* data_ptr() const { return data.data(); }

    // 添加获取维度的方法
    size_t get_Lt() const { return Lt; }
    size_t get_Lz() const { return Lz; }
    size_t get_Ly() const { return Ly; }
    size_t get_Lx() const { return Lx; }
    size_t get_Nc() const { return Nc; }
    static constexpr size_t get_Ndim() { return Ndim; }

private:
    size_t index(size_t dim, size_t t, size_t z, size_t y, 
                 size_t x, size_t c1, size_t c2) const {
        assert(dim < Ndim && t < Lt && z < Lz && y < Ly && 
               x < Lx && c1 < Nc && c2 < Nc);
        return ((((((dim * Lt + t) * Lz + z) * Ly + y) * Lx + x) * Nc + c1) * Nc + c2);
    }
};

int main() {
    try {
        // 定义数组维度
        const size_t lt = 4, lz = 4, ly = 4, lx = 4, nc = 3;
        Array7D<double> array7d(lt, lz, ly, lx, nc);

        // 初始化数据
        for (size_t dim = 0; dim < array7d.get_Ndim(); ++dim) {
            for (size_t t = 0; t < lt; ++t) {
                for (size_t z = 0; z < lz; ++z) {
                    for (size_t y = 0; y < ly; ++y) {
                        for (size_t x = 0; x < lx; ++x) {
                            for (size_t c1 = 0; c1 < nc; ++c1) {
                                for (size_t c2 = 0; c2 < nc; ++c2) {
                                    array7d(dim, t, z, y, x, c1, c2) = 
                                        static_cast<double>(dim + t + z + y + x + c1 + c2);
                                }
                            }
                        }
                    }
                }
            }
        }

        const std::string filename = "test_7d.hdf5";
        const std::string dataset_name = "LatticeMatrix";

        // 创建文件
        H5::H5File file(filename, H5F_ACC_TRUNC);

        // 定义HDF5数据维度
        std::vector<hsize_t> dims = {
            array7d.get_Ndim(), lt, lz, ly, lx, nc, nc
        };

        // 创建数据空间
        H5::DataSpace dataspace(dims.size(), dims.data());

        // 创建数据集
        H5::DataSet dataset = file.createDataSet(
            dataset_name,
            H5::PredType::NATIVE_DOUBLE,
            dataspace
        );

        // 写入数据
        dataset.write(array7d.data_ptr(), H5::PredType::NATIVE_DOUBLE);

        // 添加属性
        H5::DataSpace attr_dataspace(H5S_SCALAR);
        
        // 创建并写入各个维度的属性
        double Lt = 1.0, Lz = 2.0, Ly = 3.0, Lx = 4.0;
        
        H5::Attribute attr_Lt = dataset.createAttribute("Lt", H5::PredType::NATIVE_DOUBLE, attr_dataspace);
        attr_Lt.write(H5::PredType::NATIVE_DOUBLE, &Lt);
        
        H5::Attribute attr_Lz = dataset.createAttribute("Lz", H5::PredType::NATIVE_DOUBLE, attr_dataspace);
        attr_Lz.write(H5::PredType::NATIVE_DOUBLE, &Lz);
        
        H5::Attribute attr_Ly = dataset.createAttribute("Ly", H5::PredType::NATIVE_DOUBLE, attr_dataspace);
        attr_Ly.write(H5::PredType::NATIVE_DOUBLE, &Ly);
        
        H5::Attribute attr_Lx = dataset.createAttribute("Lx", H5::PredType::NATIVE_DOUBLE, attr_dataspace);
        attr_Lx.write(H5::PredType::NATIVE_DOUBLE, &Lx);

        // 添加Nc属性
        double Nc = static_cast<double>(nc);
        H5::Attribute attr_Nc = dataset.createAttribute("Nc", H5::PredType::NATIVE_DOUBLE, attr_dataspace);
        attr_Nc.write(H5::PredType::NATIVE_DOUBLE, &Nc);

        std::cout << "数据和属性已成功写入到" << filename << "文件中\n";
        
        return 0;
    } catch (const H5::Exception& e) {
        std::cerr << "HDF5错误：" << e.getCDetailMsg() << '\n';
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "错误：" << e.what() << '\n';
        return 1;
    }
}