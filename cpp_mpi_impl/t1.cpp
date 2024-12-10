#include <H5Cpp.h>
#include <vector>
#include <iostream>
#include <array>
#include <string>
#include <stdexcept>

int main() {
    try {
        // 定义数据维度
        const std::array<hsize_t, 4> dims = {4, 4, 4, 4};  // {nt, nz, ny, nx}
        const std::string filename = "test_4d.hdf5";
        const std::string dataset_name = "data";
        
        // 创建并初始化数据
        const size_t total_size = dims[0] * dims[1] * dims[2] * dims[3];
        std::vector<double> data(total_size);
        for (size_t i = 0; i < data.size(); ++i) {
            data[i] = static_cast<double>(i);
        }
        
        // 创建文件
        H5::H5File file(filename, H5F_ACC_TRUNC);
        
        // 创建数据空间
        H5::DataSpace dataspace(dims.size(), dims.data());
        
        // 创建数据集
        H5::DataSet dataset = file.createDataSet(
            dataset_name,
            H5::PredType::NATIVE_DOUBLE,
            dataspace
        );
        
        // 写入数据
        dataset.write(data.data(), H5::PredType::NATIVE_DOUBLE);
        
        // 文件会在作用域结束时自动关闭
        std::cout << "数据已成功写入到" << filename << "文件中\n";
        
        return 0;
    } catch (const H5::Exception& e) {
        std::cerr << "HDF5错误：" << e.getCDetailMsg() << '\n';
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "错误：" << e.what() << '\n';
        return 1;
    }
}