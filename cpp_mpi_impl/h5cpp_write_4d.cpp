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

        // 添加属性
        double Lt = 1.0, Lz = 2.0, Ly = 3.0, Lx = 4.0;
        
        // 创建标量数据空间
        H5::DataSpace attr_dataspace(H5S_SCALAR);
        
        // 创建并写入Lt属性
        H5::Attribute attr_Lt = dataset.createAttribute("Lt", H5::PredType::NATIVE_DOUBLE, attr_dataspace);
        attr_Lt.write(H5::PredType::NATIVE_DOUBLE, &Lt);
        
        // 创建并写入Lz属性
        H5::Attribute attr_Lz = dataset.createAttribute("Lz", H5::PredType::NATIVE_DOUBLE, attr_dataspace);
        attr_Lz.write(H5::PredType::NATIVE_DOUBLE, &Lz);
        
        // 创建并写入Ly属性
        H5::Attribute attr_Ly = dataset.createAttribute("Ly", H5::PredType::NATIVE_DOUBLE, attr_dataspace);
        attr_Ly.write(H5::PredType::NATIVE_DOUBLE, &Ly);
        
        // 创建并写入Lx属性
        H5::Attribute attr_Lx = dataset.createAttribute("Lx", H5::PredType::NATIVE_DOUBLE, attr_dataspace);
        attr_Lx.write(H5::PredType::NATIVE_DOUBLE, &Lx);
        
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