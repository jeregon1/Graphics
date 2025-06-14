#pragma once

#include <string>

// Functions from test_geometry.cpp
void test_translate();
void test_rotate_x();
void test_rotate_y();
void test_rotate_z();
void test_scale();
void run_geometry_tests();

// Functions from test_intersect.cpp
void testSphereIntersection();
void testPlaneIntersection();
void testTriangleIntersection();
void testConeIntersection();
void test_all_intersections();

// Functions from test_p2.cpp (Image & ToneMapping)
void test_readWritePPM(const std::string& file);
void testClamp(const std::string& path);
void testEqualization(const std::string& path);
void testEqualizationClamp(const std::string& path);
void testGamma(const std::string& path);
void testClampGamma(const std::string& path);
void test_toneMapping(const std::string& path);
void test_readWriteBMP(const std::string& file);
void test_all_image_operations();

// Functions from test_bmp.cpp
void test_bmp_format();

// Functions from test_cornell_box.cpp
void test_cornell_box_rendering();

// Functions from test_parallel.cpp
void test_parallel_rendering();
