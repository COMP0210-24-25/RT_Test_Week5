#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>
#include "VectorUtils.hpp"
#include "Rendering.hpp"
#include <cmath> 
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include "SceneLoader.hpp"
#include <memory>
#include "Object.hpp"

using namespace Catch::Matchers;

std::vector<std::vector<std::array<float, 3>>> loadImage(std::string filename, uint w = 100, uint h = 100)
{
    std::vector<std::vector<std::array<float, 3>>> image;
    std::ifstream image_file;
    image_file.open(filename);
    if (image_file)
    {
        std::string line;

        // ignore header line
        std::getline(image_file, line);

        // get dimensions
        std::getline(image_file, line);
        std::istringstream line_stream(line);
        uint width, height;
        line_stream >> width;
        line_stream >> height;

        if ((width != w) || (height != h))
        {
            throw std::runtime_error("Dimensions of the image are not as expected");
        }

        image.resize(width);
        for (auto &col : image)
        {
            col.resize(height);
        }

        // ignore  pixel limit
        std::getline(image_file, line);

        for (uint y = 0; y < height; y++)
        {
            for (uint x = 0; x < width; x++)
            {
                if (std::getline(image_file, line))
                {
                    std::istringstream pixel_stream(line);
                    pixel_stream >> image[x][y][0];
                    pixel_stream >> image[x][y][1];
                    pixel_stream >> image[x][y][2];
                }
                else
                {
                    throw std::runtime_error("Ran out of pixel data.");
                }
            }
        }
    }
    else
    {
        throw std::runtime_error("File " + filename + " not found.");
    }

    return image;
}

float clamp0to255(float val)
{
    return std::max(std::min(val, float(255.)), float(0.));
}

double diffImage(std::vector<std::vector<std::array<float, 3>>> im1,
                 std::vector<std::vector<std::array<float, 3>>> im2)
{
    size_t width, height;
    width = im1.size();
    height = im1.at(0).size();

    REQUIRE(im2.size() == width);
    REQUIRE(im2.at(0).size() == height);

    double diff = 0;
    for (size_t i = 0; i < width; i++)
    {
        for (size_t j = 0; j < width; j++)
        {
            diff += fabs(im1[i][j][0] - clamp0to255(im2[i][j][0]));
            diff += fabs(im1[i][j][1] - clamp0to255(im2[i][j][1]));
            diff += fabs(im1[i][j][2] - clamp0to255(im2[i][j][2]));
        }
    }
    double average_diff = diff / (width * height);
    return average_diff;
}

TEST_CASE("Test Empty Load", "[Test Loader]")
{
    auto scene = SceneLoader("data/Empty.txt");
    REQUIRE(scene.empty());
}

TEST_CASE("Test Plane Load", "[Test Loader]")
{
    auto scene = SceneLoader("data/SinglePlane.txt");
    Camera cam(100, 100);
    auto image = Render::genImage(cam, scene);
    
    auto expectation = loadImage("data/SinglePlane.pbm", 100, 100);

    double average_diff = diffImage(expectation, image);
    REQUIRE(average_diff < 10);
}

TEST_CASE("Test Plane (no material) Load", "[Test Loader]")
{
    auto scene = SceneLoader("data/SinglePlaneNoMaterial.txt");
    Camera cam(100, 100);
    auto image = Render::genImage(cam, scene);
    
    auto expectation = loadImage("data/SinglePlaneDefaultMaterial.pbm", 100, 100);

    double average_diff = diffImage(expectation, image);
    REQUIRE(average_diff < 10);
}

TEST_CASE("Test Sphere Load", "[Test Loader]")
{
    auto scene = SceneLoader("data/SingleSphere.txt");
    Camera cam(100, 100);
    auto image = Render::genImage(cam, scene);
    
    auto expectation = loadImage("data/SingleSphere.pbm", 100, 100);

    double average_diff = diffImage(expectation, image);
    REQUIRE(average_diff < 10);
}

TEST_CASE("Test Sphere (No Material) Load", "[Test Loader]")
{
    auto scene = SceneLoader("data/SingleSphereNoMaterial.txt");
    Camera cam(100, 100);
    auto image = Render::genImage(cam, scene);
    
    auto expectation = loadImage("data/SingleSphereDefaultMaterial.pbm", 100, 100);

    double average_diff = diffImage(expectation, image);
    REQUIRE(average_diff < 10);
}

TEST_CASE("Test Multi Object Scene", "[Test Loader]")
{
    auto scene = SceneLoader("data/Scene.txt");
    Camera cam(100, 100);
    auto image = Render::genImage(cam, scene);
    
    auto expectation = loadImage("data/Scene.pbm", 100, 100);

    double average_diff = diffImage(expectation, image);
    REQUIRE(average_diff < 10);
}

TEST_CASE("Test Ill Formatted Files", "[Test Error Handling]")
{
    REQUIRE_THROWS(SceneLoader("data/UnknownObjectType.txt"));
    REQUIRE_THROWS(SceneLoader("data/MissingInfoSphere.txt"));
    REQUIRE_THROWS(SceneLoader("data/MissingInfoPlane.txt"));
    REQUIRE_THROWS(SceneLoader("data/UndersizedTriple.txt"));
    REQUIRE_THROWS(SceneLoader("data/OversizedTriple.txt"));
    REQUIRE_THROWS(SceneLoader("data/InvalidNumericConversion.txt"));
}