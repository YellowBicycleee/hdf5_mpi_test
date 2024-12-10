#include <hdf5.h>
#include <vector>
#include <iostream>
#include <array>
#include <string>
#include <functional>
#include <stdexcept>

// RAII 包装器
class Hdf5Handle {
    hid_t id_;
    std::function<herr_t(hid_t)> closer_;
public:
    Hdf5Handle(hid_t id, std::function<herr_t(hid_t)> closer) 
        : id_(id), closer_(closer) {
        if (id_ < 0) throw std::runtime_error("HDF5 操作失败");
    }
    ~Hdf5Handle() { if (id_ >= 0) closer_(id_); }
    operator hid_t() const { return id_; }
};

int main() {
    try {
        // 定义数据维度
        const std::array<hsize_t, 4> dims = {4, 4, 4, 4};  // {nt, nz, ny, nx}
        const std::string filename = "test_4d.hdf5";
        const std::string dataset = "/data";
        
        // 创建并初始化数据
        const size_t total_size = dims[0] * dims[1] * dims[2] * dims[3];
        std::vector<double> data(total_size);
        for (size_t i = 0; i < data.size(); ++i) {
            data[i] = static_cast<double>(i);
        }
        
        // 使用 RAII 方式创建文件和数据集，明确指定类型
        Hdf5Handle file(
            H5Fcreate(filename.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT),
            H5Fclose);
        Hdf5Handle space(
            H5Screate_simple(dims.size(), dims.data(), nullptr),
            H5Sclose);
        Hdf5Handle dataset_id(
            H5Dcreate2(file, dataset.c_str(), H5T_NATIVE_DOUBLE, space,
                      H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT),
            H5Dclose);
        
        // 写入数据
        if (H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL,
                     H5P_DEFAULT, data.data()) < 0) {
            throw std::runtime_error("写入数据失败");
        }

        std::cout << "数据已成功写入到" << filename << "文件中\n";
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "错误：" << e.what() << '\n';
        return 1;
    }
}