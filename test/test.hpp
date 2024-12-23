#pragma once

#include <string>

using 

// Functions from test_geometry.cpp
void test_translate();
void test_rotate_x();
void test_rotate_y();
void test_rotate_z();
void test_scale();

// Functions from test_p2.cpp
void test_readWritePPM(const std::string& file);
void testClamp(const std::string& path);
void testEqualization(const std::string& path);
void testEqualizationClamp(const std::string& path);
void testGamma(const std::string& path);
void testClampGamma(const std::string& path);
void test_toneMapping(const std::string& path);
void test_readWriteBMP(const std::string& file);

// Functions from test_p3.cpp
// Add function declarations here when available
