
#include "Image.hpp"

void skipComments(ifstream &file, float &maxColor) {
    while (file.peek() == '#') {
        string comment;
        getline(file, comment);
        if (comment.find("#MAX=") == 0) {
            maxColor = stof(comment.substr(5));
        }
    }
}

Image Image::readPPM(string filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file" << endl;
        return Image();
    }

    float maxColor = 0;

    skipComments(file, maxColor);
    string line;
    getline(file, line);
    if (line != "P3") {
        cerr << "Invalid PPM format in file " + filename << endl;
        return Image();
    }

    skipComments(file, maxColor);
    int width, height;
    file >> width >> height;

    skipComments(file, maxColor);
    if (maxColor == 0)
        file >> maxColor;
    
    skipComments(file, maxColor);

    vector<RGB> pixels(width * height);
    int r, g, b;
    for (int i = 0; i < width * height; i++) {
        file >> r >> g >> b;
        pixels[i] = RGB(r / maxColor, g / maxColor, b / maxColor);
    }
    file.close();
    return Image(width, height, pixels);
}

void Image::writePPM(string filename, float maxColor) {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file " + filename << endl;
        return;
    }
    file << "P3\n";
    file << width << " " << height << "\n";
    file << (int) maxColor << "\n";
    for (int i = 0; i < height; i++) {
        int j = i * width;
        file << pixels[j].r << " " << pixels[j].g << " " << pixels[j].b; // First element without \t
        for (j = j + 1; j < (i + 1) * width; j++) {
            file << "\t" << pixels[j].r << " " << pixels[j].g << " " << pixels[j].b;
        }
        file << "\n";
    }

    file.close();
}