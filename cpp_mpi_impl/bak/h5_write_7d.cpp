#include <H5Cpp.h>
#include <vector>
#include <iostream>
#include <array>
#include <string>
#include <stdexcept>
#include <complex>
#include <cassert>

#include "public.h"

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    try {
        // 解析命令行参数 (x,y,z,t顺序)
        if (argc != 2) {
            if (rank == 0) {
                std::cerr << "用法: mpirun -n <进程数> " << argv[0] << " Nx.Ny.Nz.Nt\n";
            }
            MPI_Finalize();
            return 1;
        }

        // 解析进程网格
        std::string grid_str(argv[1]);
        int nx, ny, nz, nt;
        size_t pos = 0;
        nx = std::stoi(grid_str, &pos); grid_str = grid_str.substr(pos + 1);
        ny = std::stoi(grid_str, &pos); grid_str = grid_str.substr(pos + 1);
        nz = std::stoi(grid_str, &pos); grid_str = grid_str.substr(pos + 1);
        nt = std::stoi(grid_str);

        if (nx * ny * nz * nt != size) {
            if (rank == 0) {
                std::cerr << "错误：进程网格大小 (" << nx << "," << ny << "," 
                         << nz << "," << nt << ") 与总进程数 " << size << " 不匹配\n";
            }
            MPI_Finalize();
            return 1;
        }

        // 设置全局和局部数组大小
        const size_t Lt = 8, Lz = 8, Ly = 8, Lx = 8, Nc = 3;  // 全局大小
        
        // 确保可以整除
        if (Lt % nt != 0 || Lz % nz != 0 || Ly % ny != 0 || Lx % nx != 0) {
            if (rank == 0) {
                std::cerr << "错误：网格维度必须能被进程数整除\n";
            }
            MPI_Finalize();
            return 1;
        }

        // 计算局部大小
        const size_t local_lt = Lt / nt;
        const size_t local_lz = Lz / nz;
        const size_t local_ly = Ly / ny;
        const size_t local_lx = Lx / nx;

        // 计算当前进程在4D网格中的位置
        Coords coords;
        coords.data[T_DIM] = rank / (nx * ny * nz);  // t
        int remainder = rank % (nx * ny * nz);
        coords.data[Z_DIM] = remainder / (nx * ny);   // z
        remainder = remainder % (nx * ny);
        coords.data[Y_DIM] = remainder / nx;          // y
        coords.data[X_DIM] = remainder % nx;          // x

        // 计算局部数组在全局数组中的偏移
        const size_t offset_t = coords.T() * local_lt;
        const size_t offset_z = coords.Z() * local_lz;
        const size_t offset_y = coords.Y() * local_ly;
        const size_t offset_x = coords.X() * local_lx;

        // 创建局部数组
        Array7D<std::complex<double>> local_array(local_lt, local_lz, local_ly, local_lx, Nc);

        // 初始化局部数据
        constexpr int MAX_ELEM = 9 * 32;
        int counter = 0;

        // 初始化数据
        for (size_t dim = 0; dim < local_array.get_Ndim(); ++dim) {
            for (size_t t = 0; t < local_lt; ++t) {
                for (size_t z = 0; z < local_lz; ++z) {
                    for (size_t y = 0; y < local_ly; ++y) {
                        for (size_t x = 0; x < local_lx; ++x) {
                            for (size_t c1 = 0; c1 < Nc; ++c1) {
                                for (size_t c2 = 0; c2 < Nc; ++c2) {
                                    double temp = rank * 1000 + static_cast<double>(counter++);
                                    counter = counter % MAX_ELEM;
                                    local_array(dim, t, z, y, x, c1, c2) = {temp, temp + 0.1};
                                }
                            }
                        }
                    }
                }
            }
        }

        // HDF5并行写入
        {
            const std::string filename = "test_7d.hdf5";
            const std::string dataset_name = "LatticeMatrix";

            // 设置并行访问属性
            H5::FileAccPropList plist;
            plist.copy(H5::FileAccPropList::DEFAULT);
            H5Pset_fapl_mpio(plist.getId(), MPI_COMM_WORLD, MPI_INFO_NULL);

            // 创建文件
            H5::H5File file(filename, H5F_ACC_TRUNC, H5::FileCreatPropList::DEFAULT, plist);

            // 创建全局数据空间
            std::vector<hsize_t> dims = {
                local_array.get_Ndim(), Lt, Lz, Ly, Lx, Nc, Nc * 2
            };
            H5::DataSpace filespace(dims.size(), dims.data());

            // 创建数据集
            H5::DataSet dataset = file.createDataSet(dataset_name, 
                H5::PredType::NATIVE_DOUBLE, filespace);

            // 设置局部数据空间
            std::vector<hsize_t> local_dims = {
                local_array.get_Ndim(), local_lt, local_lz, local_ly, local_lx, Nc, Nc * 2
            };
            std::vector<hsize_t> offset = {
                0, offset_t, offset_z, offset_y, offset_x, 0, 0
            };
            
            H5::DataSpace memspace(local_dims.size(), local_dims.data());
            filespace.selectHyperslab(H5S_SELECT_SET, local_dims.data(), offset.data());

            // 设置集体写入属性
            H5::DSetMemXferPropList xfer_plist;
            xfer_plist.copy(H5::DSetMemXferPropList::DEFAULT);
            H5Pset_dxpl_mpio(xfer_plist.getId(), H5FD_MPIO_COLLECTIVE);

            // 写入数据
            dataset.write(local_array.data_ptr(), H5::PredType::NATIVE_DOUBLE, 
                         memspace, filespace, xfer_plist);
        }

        MPI_Finalize();
        return 0;
    } catch (const H5::Exception& e) {
        if (rank == 0) {
            std::cerr << "HDF5错误：" << e.getCDetailMsg() << '\n';
        }
        MPI_Finalize();
        return 1;
    } catch (const std::exception& e) {
        if (rank == 0) {
            std::cerr << "错误：" << e.what() << '\n';
        }
        MPI_Finalize();
        return 1;
    }
}