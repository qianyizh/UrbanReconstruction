Dual Contouring Demo
Qian-Yi Zhou (qianyizh@usc.edu)
Version: 1.01, last updated 10/5/2010
Project page: 
http://graphics.usc.edu/~qianyizh/projects/dualcontouring.html
http://graphics.usc.edu/~qianyizh/projects/buildingreconstruction.html
http://graphics.usc.edu/~qianyizh/software.html


I. Introduction

This program is a demo of 2.5D dual contouring, based on paper '2.5D Dual Contouring: A Robust Approach to Creating Building Models from Aerial LiDAR Point Clouds', published on ECCV 2010.
All the related resources can be found on the project page. 


II. Usage

The configuration of the demo is stored in config.ini which should be put in the same directory of the executable program.

In the configuration file, items mean:

[DualContouring]
WorkingDir ="..\models\"
	-- Working directory, in which the program will traverse all the files ending in "xyzn" and apply our algorithm to produce "obj" files
GridLength =1.0
	-- Unit length of the uniform grid
AcceptNumber =4
	-- During segmentation, this is the minimum point number of a potential roof cluster
RelativeDistance =100.0
RelativeZ =1.0
	-- Segmentation takes two points in the same cluster iff ( distance < RelativeDistance && height_difference < RelativeZ )
Weight =1.0
	-- Weight w to balance 2D boundary error term and 3D surface error term
ErrorTolerance =1.0
	-- Threshold used to control bottom-up simplification
SingularTolerance =0.15
	-- Threshold used to determine if singular treatment is needed when solving matrix
WallRectangle =0
AntiNonManifold =1
SnappingErrorTolerance =1.2
SnappingMinimumLength =7.0
	-- Advanced functions not supported in this demo
	
******** for a fully functional program, visit our website ********
http://graphics.usc.edu/~qianyizh/projects/buildingreconstruction.html


III. Release Notes

1.01
	Fixed a bug that may cause program crash.

1.00
	A first console executable file, containing all the basic functions.
