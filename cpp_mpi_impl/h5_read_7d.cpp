#include <H5Cpp.h>
#include <vector>
#include <iostream>
#include <array>
#include <string>
#include <stdexcept>
#include <complex>

#include "public.h"

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    try {
        // 解析命令行参数
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
                std::cerr << "错误：进程网格大小与总进程数不匹配\n";
            }
            MPI_Finalize();
            return 1;
        }

        const std::string filename = "test_7d.hdf5";
        const std::string dataset_name = "LatticeMatrix";

        {
            // 设置并行访问属性
            H5::FileAccPropList plist;
            plist.copy(H5::FileAccPropList::DEFAULT);
            H5Pset_fapl_mpio(plist.getId(), MPI_COMM_WORLD, MPI_INFO_NULL);

            // 打开HDF5文件
            H5::H5File file(filename, H5F_ACC_RDONLY, H5::FileCreatPropList::DEFAULT, plist);
            H5::DataSet dataset = file.openDataSet(dataset_name);

            // 获取数据空间和维度信息
            H5::DataSpace dataspace = dataset.getSpace();
            const int ndims = dataspace.getSimpleExtentNdims();
            std::vector<hsize_t> dims(ndims);
            dataspace.getSimpleExtentDims(dims.data(), nullptr);

            // 计算局部大小
            const size_t local_lt = dims[1] / nt;
            const size_t local_lz = dims[2] / nz;
            const size_t local_ly = dims[3] / ny;
            const size_t local_lx = dims[4] / nx;
            const size_t Nc = dims[5];

            // 计算当前进程在4D网格中的位置
            Coords coords;
            int remainder;
            coords.data[T_DIM] = rank / (nx * ny * nz);  remainder = rank % (nx * ny * nz); // t
            coords.data[Z_DIM] = remainder / (nx * ny);  remainder = remainder % (nx * ny); // z
            coords.data[Y_DIM] = remainder / nx;         remainder = remainder % nx; // y
            coords.data[X_DIM] = remainder;                 // x

            // 计算偏移量
            const size_t offset_t = coords.T() * local_lt;
            const size_t offset_z = coords.Z() * local_lz;
            const size_t offset_y = coords.Y() * local_ly;
            const size_t offset_x = coords.X() * local_lx;

            // 创建局部数组
            Array7D<std::complex<double>> local_array(local_lt, local_lz, local_ly, local_lx, Nc);

            // 设置局部数据空间
            std::vector<hsize_t> local_dims = {
                local_array.get_Ndim(), local_lt, local_lz, local_ly, local_lx, Nc, Nc * 2
            };
            std::vector<hsize_t> offset = {
                0, offset_t, offset_z, offset_y, offset_x, 0, 0
            };

            H5::DataSpace memspace(local_dims.size(), local_dims.data());
            dataspace.selectHyperslab(H5S_SELECT_SET, local_dims.data(), offset.data());

            // 设置集体读取属性
            H5::DSetMemXferPropList xfer_plist;
            xfer_plist.copy(H5::DSetMemXferPropList::DEFAULT);
            H5Pset_dxpl_mpio(xfer_plist.getId(), H5FD_MPIO_COLLECTIVE);

            // 读取数据
            dataset.read(reinterpret_cast<double*>(local_array.data_ptr()), 
                        H5::PredType::NATIVE_DOUBLE, memspace, dataspace, xfer_plist);

            // 按进程顺序输出每个进程的两个Nc * Nc矩阵
            for (int current_rank = 0; current_rank < size; ++current_rank) {
                if (rank == current_rank) {
                    std::cout << "\n进程 " << rank << " 的两个 Nc * Nc 矩阵:\n";
                    // 固定其他维度,输出两个color矩阵
                    const size_t t = 0, z = 0, y = 0, x = 0;
                    // 输出前两个维度
                    for (size_t dim = 0; dim < 2; ++dim) {
                        std::cout << "维度 " << dim << ":\n";
                        for (size_t c1 = 0; c1 < Nc; ++c1) {
                            for (size_t c2 = 0; c2 < Nc; ++c2) {
                                std::cout << local_array(dim, t, z, y, x, c1, c2) << "\t";
                            }
                            std::cout << "\n";
                        }
                        std::cout << "\n";
                    }
                    std::cout << std::flush;

                    {
                        std::cout << "last matrix " << ":\n";
                        for (size_t c1 = 0; c1 < Nc; ++c1) {
                            for (size_t c2 = 0; c2 < Nc; ++c2) {
                                std::cout << local_array(3, local_lt - 1, local_lz - 1, local_ly - 1, local_lx - 1, c1, c2) << "\t";
                            }
                            std::cout << "\n";
                        }
                        std::cout << "\n";
                    }
                }
                MPI_Barrier(MPI_COMM_WORLD); // 确保按顺序输出
            }
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