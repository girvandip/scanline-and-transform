#ifndef POLYGON
#define POLYGON

#include "../line/line.cpp"
#include <vector>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <utility>
#include <math.h>
#include <algorithm>
#include "../utils/util.cpp"
#include "../clipping/clip.cpp"

using namespace std;

class Polygon {
    public:
        Polygon() {

        }

        ~Polygon(){
        }
        /* Open a file with the fileName */
        Polygon(char * fileName) {
            FILE *file;
            file = fopen(fileName, "r");

            int x1, y1, x2, y2;
            int numberOfLines;
            fscanf(file, "%d", &numberOfLines);

            for(int i = 0; i < numberOfLines; i++){

                fscanf(file, "%d %d %d %d", &x1, &y1, &x2, &y2);

                Point p1(x1, y1);
                Point p2(x2, y2);
                Line temp(p1, p2);
                lines.push_back(temp);
            }

            // get the max point
            fscanf(file, "%d %d", &x1, &y1);
            topLeft.setAxis(x1);
            topLeft.setOrdinat(y1);
            fscanf(file, "%d %d", &x1, &y1);
            bottomRight.setAxis(x1);
            bottomRight.setOrdinat(y1);
        }

        void print(int divx, int divy, int red, int green, int blue, Clip clip, int*** buffer) {
            for(int i = 0; i < lines.size(); i++){
                Line temp(
                    lines[i].getFirstPoint().getAxis(),
                    lines[i].getFirstPoint().getOrdinat(),
                    lines[i].getSecondPoint().getAxis(),
                    lines[i].getSecondPoint().getOrdinat()
                );
                bool print = clip.clipLine(temp);
                if (print) temp.print(divx, divy, red, green, blue, buffer);
            }
        }

        Point getTopLeft() {
            return topLeft;
        }
        Point getBottomRight() {
            return bottomRight;
        }

        // TODO: make line dependent scan line
        void scanLine(int red, int green, int blue, Clip clipBorder, int*** buffer) {
            // make the scan line
            int scanLineY = topLeft.getOrdinat(); // Represent Y = c
            
            for(int scan = topLeft.getOrdinat(); scan < bottomRight.getOrdinat(); scan++){
                // list of points
                vector<Point> listOfIntersectPoints;

                for(int i = 0; i < lines.size(); i++){
                    // First, check if the point intersect
                    int firstOrdinatPolygon = lines[i].getFirstPoint().getOrdinat();
                    int secondOrdinatPolygon = lines[i].getSecondPoint().getOrdinat();

                    if((scanLineY - firstOrdinatPolygon)*(scanLineY - secondOrdinatPolygon) <= 0){
                        // find the intersect
                        pair<float, float> tempEQ = lines[i].makeLine();

                        if(tempEQ.first == 0){
                            Point tempPoint(lines[i].getFirstPoint().getAxis(), scanLineY);
                            listOfIntersectPoints.push_back(tempPoint);
                        } else {
                            float x = (scanLineY - tempEQ.second) / tempEQ.first;
                            // float y = linearEQ.first * x + linearEQ.second;
                            Point tempPoint(x, scanLineY);
                            listOfIntersectPoints.push_back(tempPoint);
                        }
                    }

                    // check if it has a corner
                    for(int j = i + 1; j < lines.size(); j++){
                        if(isCorner(lines[i], lines[j], scanLineY)){
                            // add another point
                            pair<float, float> tempEQ = lines[i].makeLine();

                            if(tempEQ.first == 0){
                                Point tempPoint(lines[i].getFirstPoint().getAxis(), scanLineY);
                                listOfIntersectPoints.push_back(tempPoint);
                            } else {
                                float x = (scanLineY - tempEQ.second) / tempEQ.first;
                                Point tempPoint(x, scanLineY);
                                listOfIntersectPoints.push_back(tempPoint);
                            }
                        }
                    }
                }

                // sort the points
                if(listOfIntersectPoints.size() != 0){
                    for(int i = 0; i < listOfIntersectPoints.size() - 1; i++) {
                        int smallest = i;
                        for(int j = i+1; j < listOfIntersectPoints.size(); j++){
                            if(listOfIntersectPoints[j].getAxis() < listOfIntersectPoints[smallest].getAxis()){
                                smallest = j;
                            }
                        }
                        Point temp(listOfIntersectPoints[smallest].getAxis(), listOfIntersectPoints[smallest].getOrdinat());
                        listOfIntersectPoints[smallest] = listOfIntersectPoints[i];
                        listOfIntersectPoints[i] = temp;
                    }

                    // only color if total point is more than one
                    if(listOfIntersectPoints.size() > 1){
                        // check if the count is odd
                        int count = (listOfIntersectPoints.size() % 2 == 1) ?
                            (listOfIntersectPoints.size() - 1) : listOfIntersectPoints.size();

                        for(int i = 0; i < count; i++) {
                            // don't color the border
                            listOfIntersectPoints[i].setAxis(listOfIntersectPoints[i].getAxis()+1);
                            listOfIntersectPoints[i+1].setAxis(listOfIntersectPoints[i+1].getAxis()-1);
                            
                            Line line(listOfIntersectPoints[i], listOfIntersectPoints[i+1]);
                            //cout << "lama :" << line.getFirstPoint().getAxis() << " " << line.getSecondPoint().getOrdinat() << " ";
                            //if(scan == 501) cout << "yo nyampe" << endl;
                            bool print = clipBorder.clipLine(line);
                            //cout << "baru: " << line.getFirstPoint().getAxis() << " " << line.getSecondPoint().getOrdinat() << " ";
                            // if(scan == 501){
                            //     cout << "sudah dapet print " << print << endl;
                            //     cout << line.getFirstPoint().getAxis() << " " << line.getFirstPoint().getOrdinat() << " "
                            //         << line.getSecondPoint().getAxis() << " " << line.getSecondPoint().getOrdinat() << endl;
                            // } 
                            if(print)
                                line.print(0,0, red, green, blue, buffer);

                            i++;
                            //if(scan == 501) cout << "kelar" << endl;
                            
                        }
                    }
                }

                scanLineY++;
            }
        }

        void fall(){
			int minX = topLeft.getAxis(),
                minY = topLeft.getOrdinat(),
                maxX = bottomRight.getAxis(),
                maxY = bottomRight.getOrdinat();

			for(int i = 0;i < lines.size();i++) {
                lines[i].move(0,1, topLeft, bottomRight);
            }
            topLeft.setAxis(minX);
            topLeft.setOrdinat(minY+1);
            bottomRight.setAxis(maxX);
            bottomRight.setOrdinat(maxY+1);


		}

		void up(){
			int minX = topLeft.getAxis(),
                minY = topLeft.getOrdinat(),
                maxX = bottomRight.getAxis(),
                maxY = bottomRight.getOrdinat();

			for(int i = 0;i < lines.size();i++) {
                lines[i].move(0,-1, topLeft, bottomRight);
            }
            topLeft.setAxis(minX);
            topLeft.setOrdinat(minY-1);
            bottomRight.setAxis(maxX);
            bottomRight.setOrdinat(maxY-1);


		}


        void rotate(int degree) {

            for(int i = 0;i < lines.size();i++) {
                lines[i].rotate(degree, topLeft, bottomRight);
            }

            // Update the topLeft and topRight
            int minX = topLeft.getAxis(),
                minY = topLeft.getOrdinat(),
                maxX = bottomRight.getAxis(),
                maxY = bottomRight.getOrdinat();
            for(int i = 0; i < lines.size(); i++){
                if(lines[i].getFirstPoint().getAxis() < minX){
                    minX = lines[i].getFirstPoint().getAxis();
                }
                if(lines[i].getFirstPoint().getOrdinat() < minY){
                    minY = lines[i].getFirstPoint().getOrdinat();
                }
                if(lines[i].getSecondPoint().getAxis() > maxX){
                    maxX = lines[i].getSecondPoint().getAxis();
                }
                if(lines[i].getSecondPoint().getOrdinat() > maxY){
                    maxY = lines[i].getSecondPoint().getOrdinat();
                }
            }
            topLeft.setAxis(minX);
            topLeft.setOrdinat(minY);
            bottomRight.setAxis(maxX);
            bottomRight.setOrdinat(maxY);

            // cout << topLeft.getAxis() << " " << topLeft.getOrdinat() << endl;
            // cout << bottomRight.getAxis() << " " << bottomRight.getOrdinat() << endl;
        }

        void scale(float scale) {
            int topLeftX = topLeft.getAxis();
            int topLeftY = topLeft.getOrdinat();
            int bottomRightX = bottomRight.getAxis();
            int bottomRightY = bottomRight.getOrdinat();
            topLeft.scale(scale, topLeftX, topLeftY, bottomRightX, bottomRightY);
            bottomRight.scale(scale, topLeftX, topLeftY, bottomRightX, bottomRightY);
            for(int i = 0;i < lines.size();i++) {
                lines[i].scale(scale, topLeft, bottomRight);
            }
        }

        void scaleByPoint(float scale, Point P) {
            topLeft.scaleByPoint(scale, P);
            bottomRight.scaleByPoint(scale, P);
            for(int i = 0;i < lines.size();i++) {
                lines[i].scaleByPoint(scale, P);
            }
        }

        void update(int x, int y) {
            topLeft.update(x, y);
            bottomRight.update(x, y);
            for(int i = 0; i < lines.size(); i++){
                lines[i].update(x, y);
            }
        }

        bool isCorner(Line firstLine, Line secondLine, int scanLine) {
            int sameY, result;
            if(firstLine.getFirstPoint().getOrdinat() == secondLine.getFirstPoint().getOrdinat()
                    && firstLine.getFirstPoint().getOrdinat() == scanLine
                    && firstLine.getFirstPoint().getAxis() == secondLine.getFirstPoint().getAxis()) {
                sameY = firstLine.getFirstPoint().getOrdinat();
                result = (sameY - firstLine.getSecondPoint().getOrdinat()) *
                            (sameY - secondLine.getSecondPoint().getOrdinat());
                return result < 0;
            } else
            if(firstLine.getFirstPoint().getOrdinat() == secondLine.getSecondPoint().getOrdinat()
                    && firstLine.getFirstPoint().getOrdinat() == scanLine
                    && firstLine.getFirstPoint().getAxis() == secondLine.getSecondPoint().getAxis()) {
                sameY = firstLine.getFirstPoint().getOrdinat();
                result = (sameY - firstLine.getSecondPoint().getOrdinat()) *
                            (sameY - secondLine.getFirstPoint().getOrdinat());
                return result < 0;
            } else
            if(firstLine.getSecondPoint().getOrdinat() == secondLine.getFirstPoint().getOrdinat()
                    && firstLine.getSecondPoint().getOrdinat() == scanLine
                    && firstLine.getSecondPoint().getAxis() == secondLine.getFirstPoint().getAxis()) {
                sameY = firstLine.getSecondPoint().getOrdinat();
                result = (sameY - firstLine.getFirstPoint().getOrdinat()) *
                            (sameY - secondLine.getSecondPoint().getOrdinat());
                return result < 0;
            } else
            if(firstLine.getSecondPoint().getOrdinat() == secondLine.getSecondPoint().getOrdinat()
                    && firstLine.getSecondPoint().getOrdinat() == scanLine
                    && firstLine.getSecondPoint().getAxis() == secondLine.getSecondPoint().getAxis()) {
                sameY = firstLine.getSecondPoint().getOrdinat();
                result = (sameY - firstLine.getFirstPoint().getOrdinat()) *
                            (sameY - secondLine.getFirstPoint().getOrdinat());
                return result < 0;
            }
        }

    bool isHitFromBottom(Polygon p) {
        return (this->getBottomRight().getOrdinat() > p.getTopLeft().getOrdinat());
    }

    bool isHitFromTop(Polygon p) {
        return (this->getTopLeft().getOrdinat() < p.getBottomRight().getOrdinat());
    }

    bool isHitFromRight(Polygon p) {
        return (this->getBottomRight().getAxis() > p.getTopLeft().getAxis());
    }

    bool isHitFromLeft(Polygon p) {
        return (this->getTopLeft().getAxis() < p.getBottomRight().getAxis());
    }

    bool isHitBy(Polygon p) {
        return (this->isHitFromBottom(p) && this->isHitFromLeft(p) && this->isHitFromRight(p) && this->isHitFromTop(p));
    }


    void bounce(float ratio, Clip clipBorder){
        float height = 200;
        while (height * ratio > 0) {
            for (int i = 0; i < height; ++i){
                //Util::clearScreen();

                // this->scanLine(150, 0, 150, clipBorder);
                // this->print(0, 0, 0, 255, 0, clipBorder);
                this->fall();
                this->update(1,0);
            }
            height *= ratio;
            for (int i = 0; i < height; ++i){
                //Util::clearScreen();

                // this->scanLine(150, 0, 150, clipBorder);
                // this->print(0, 0, 0, 255, 0, clipBorder);
                this->up();
                this->update(1,0);
            }
        }
    }
    private:
        vector<Line> lines;
        Point topLeft, bottomRight;
};

#endif
