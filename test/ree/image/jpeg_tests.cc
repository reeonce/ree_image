#include <iostream>

#define _USE_MATH_DEFINES
#include <math.h>

#include <ree/unittest.h>

#include <ree/image/test_config.h>
#include <ree/image/io/jpeg.hpp>
#include <ree/image/io/ppm.hpp>
#include <ree/image/process/image.hpp>

namespace ree {
namespace image {
namespace io {

void testJpeg() {
    int block[8][8] = {
        { 52, 55, 61, 66, 70, 61, 64, 73, },
        { 63, 59, 55, 90, 109, 85, 69, 72, },
        { 62, 59, 68, 113, 144, 104, 66, 73, },
        { 63, 58, 71, 122, 154, 106, 70, 69, },
        { 67, 61, 68, 104, 126, 88, 68, 70, },
        { 79, 65, 60, 70, 77, 68, 58, 75, },
        { 85, 71, 64, 59, 55, 61, 65, 83, },
        { 87, 79, 69, 68, 65, 76, 78, 94, },
    };

    int g[8][8];


    for (int x = 0; x < 8; ++x) {
        for (int y = 0; y < 8; ++y) {
            g[x][y] = block[x][y] - 128;
        }
    }


    float G[8][8];

    for (int u = 0; u < 8; ++u) {
        for (int v = 0; v < 8; ++v) {
            float au = u == 0 ? 1 / sqrt(2.0) : 1;
            float av = v == 0 ? 1 / sqrt(2.0) : 1;

            float s = 0.0;
            for (int x = 0; x < 8; ++x) {
                for (int y = 0; y < 8; ++y) {
                    s += g[x][y] * cos(((2 * x + 1) * u * M_PI) / 16.0) * 
                        cos(((2 * y + 1) * v * M_PI) / 16.0);
                }
            }
            G[u][v] = s / 4 * au * av;
        }
    }

    float g1[8][8];

    for (int x = 0; x < 8; ++x) {
        for (int y = 0; y < 8; ++y) {
            float s = 0.0;

            for (int u = 0; u < 8; ++u) {
                for (int v = 0; v < 8; ++v) {
                    float au = u == 0 ? 1 / sqrt(2.0) : 1;
                    float av = v == 0 ? 1 / sqrt(2.0) : 1;
                    s += au * av * G[u][v] * cos(((2 * x + 1) * u * M_PI) / 16.0) * 
                        cos(((2 * y + 1) * v * M_PI) / 16.0);
                }
            }
            g1[x][y] = s / 4;
        }
    }

    int Q[8][8] = {
        { 16,11,10,16,24,40,51,61, },
        { 12,12,14,19,26,58,60,55, },
        { 14,13,16,24,40,57,69,56, },
        { 14,17,22,29,51,87,80,62, },
        { 18,22,37,56,68,109,103,77, },
        { 24,35,55,64,81,104,113,92, },
        { 49,64,78,87,103,121,120,101, },
        { 72,92,95,98,112,100,103,99, },
    };

    int B[8][8];

    for (int x = 0; x < 8; ++x) {
        for (int y = 0; y < 8; ++y) {
            B[x][y] = round(G[x][y] / Q[x][y]);
        }
    }

    for (int x = 0; x < 8; ++x) {
        for (int y = 0; y < 8; ++y) {
            std::cout << B[x][y] << "\t";
        }
        std::cout << std::endl;
    }
}

R_TEST_F(Jpeg, ParseJpeg) {
    Jpeg jpeg;
    {
        auto source = ree::io::Source::SourceByPath(kTestAssetsDir + "dot1.jpg");
        source->OpenToRead();
        auto ctx = jpeg.CreateParseContext(source.get(), LoadOptions());
        Image img = jpeg.LoadImage(ctx);
        source->Close();

        
        process::Image<uint8_t> pImg = process::ImageFromIOImage<uint8_t>(img);
        process::Image<uint8_t> pCtvedImg = pImg.ConvertToColor(ColorSpace::RGB);
        Image wImg = pCtvedImg.ToIOImage();

        Ppm ppm;
        auto wsource = ree::io::Source::SourceByPath(kTestAssetsDir + "jpg_ret.ppm");
        wsource->OpenToWrite();
        auto wctx = ppm.CreateComposeContext(wsource.get(), WriteOptions());
        ppm.WriteImage(wctx, wImg);
        wsource->Close();
    }
}

}
}
}
