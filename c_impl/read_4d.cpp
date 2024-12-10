#include <hdf5.h>
#include <vector>
#include <iostream>
#include <array>
#include <string>
#include <stdexcept>
#include <functional>

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
        const std::string filename = "test_4d.hdf5";
        const std::string dataset = "/data";
        
        // 使用 RAII 方式打开文件和数据集，明确指定类型
        Hdf5Handle file(H5Fopen(filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT), H5Fclose);
        Hdf5Handle dataset_id(H5Dopen2(file, dataset.c_str(), H5P_DEFAULT), H5Dclose);
        Hdf5Handle dataspace_id(H5Dget_space(dataset_id), H5Sclose);

        // 获取维度信息
        const int ndims = H5Sget_simple_extent_ndims(dataspace_id);
        std::array<hsize_t, 4> dims;
        H5Sget_simple_extent_dims(dataspace_id, dims.data(), nullptr);

        // 分配内存并读取数据
        std::vector<double> data(dims[0] * dims[1] * dims[2] * dims[3]);
        if (H5Dread(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, 
                    H5P_DEFAULT, data.data()) < 0) {
            throw std::runtime_error("读取数据失败");
        }

        // 打印维度信息
        std::cout << "数据维度: " 
                  << dims[0] << "x" << dims[1] << "x" 
                  << dims[2] << "x" << dims[3] << '\n';

        // 查看前几个元素
        constexpr size_t num_to_print = 10;
        for (size_t i = 0; i < std::min(num_to_print, data.size()); ++i) {
            std::cout << "data[" << i << "] = " << data[i] << '\n';
        }

    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << '\n';
        return 1;
    }
    
    return 0;
}