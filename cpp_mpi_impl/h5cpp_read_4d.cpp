#include <H5Cpp.h>
#include <vector>
#include <iostream>
#include <array>
#include <string>
#include <stdexcept>

int main() {
    try {
        const std::string filename = "test_4d.hdf5";
        const std::string dataset_name = "/data";

        // 打开文件和数据集
        H5::H5File file(filename, H5F_ACC_RDONLY);
        H5::DataSet dataset = file.openDataSet(dataset_name);

        // 获取数据空间
        H5::DataSpace dataspace = dataset.getSpace();
        
        // 获取维度信息
        const int ndims = dataspace.getSimpleExtentNdims();
        std::array<hsize_t, 4> dims;
        dataspace.getSimpleExtentDims(dims.data(), nullptr);

        // 分配内存并读取数据
        std::vector<double> data(dims[0] * dims[1] * dims[2] * dims[3]);
        dataset.read(data.data(), H5::PredType::NATIVE_DOUBLE);

        // 打印维度信息
        std::cout << "数据维度: " 
                  << dims[0] << "x" << dims[1] << "x" 
                  << dims[2] << "x" << dims[3] << '\n';

        // 查看前几个元素
        constexpr size_t num_to_print = 10;
        for (size_t i = 0; i < std::min(num_to_print, data.size()); ++i) {
            std::cout << "data[" << i << "] = " << data[i] << '\n';
        }

    } catch (const H5::Exception& e) {
        std::cerr << "HDF5错误: " << e.getCDetailMsg() << '\n';
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "标准错误: " << e.what() << '\n';
        return 1;
    }

    return 0;
}