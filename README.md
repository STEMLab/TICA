# TICA v0.2.2

TICA is a cleaning & authoring tool for TM-based pipeline to construct indoor space models.

## Main features

- Editing polygons (e.g. edit polygons, creating rooms, walls)
- Annotating indoor features (doors, public safety features)

## Prerequisites

- OpenGL, GLEW, GLFW, libjpeg, ImGui
- Tested on Visual Studio 2017 (Windows)

## Quick Start

- Install depdendencies with vcpkg
	- vcpkg install opengl glew glfw3 imgui libjpeg-turbo boost-uuid
- Generate VS Project file with CMake GUI
	- Choose /src as source code folder
	- Choose /build as build folder
	- Click Configure
	- Click Generate
- Build the project
	- Open the generated project file using Visual Studio
	- Set TICAv022 as start project
	- Build
	- If build fails with linking error, please add "bcrypt.lib" into Project Property > Linker > Input > Additional Dependencies
- You can also use the pre-built binary files at /prebuilt

## Usage

- Camera Moving
	- W/S/A/D: Move forward/backward/left/right
	- T/G: Move up/down
	- Right Click: Camera rotation
	- Left Click: Select/deselect objects

- Geometry Editor
	- Edit Polygons Mode
		- To create a vertex, Click in the middle of an edge.
		- To remove a vertex, press ESC key while dragging the vertex.
		- To create a facet, select one facet before click the "Edit Polygons" menu. Then you can see the grid representing the base plane containing the selected facet. Clicking on that plane will creates a facet.
		- Click "OK" button to quit the polygon editing mode.
	- Make a room.
		- Create a floor facet in "Edit Polygon" mode, and Click "OK" button.
		- Select the created floor facet.
		- Click "Make 2.5D Solid" menu.
		- Set the height by dragging up/down on the screen.
		- Click "OK" button.
	- Make a wall.
		- Select an edge.
		- Click "Make Wall."
		- Set the height by dragging up/down on the screen.
		- Click "OK" button.
	- Make Connection between facets
		- After selecting two facets, click "Make Connection."
		- Click on a facet will create a vertex sequentially.
		- Click "OK" button to finalize the connection editining.
	- Vertex Alignment
		- To make edge (u,v) parallel to edge (x,y)
			- Select edge (x,y) and vertices u,v.
			- Click "Make parallel" menu.
		- To make edge (u,v) perpendicular to edge (x,u).
			- Select edge (x,u) and vertex v.
			- Click "Make perpendicular" menu.
		- Selected edges and vertices should be those of a facet that is not adjacent to any other.
    
- State Editor
	- After entering into the State Editor mode, the geometry becomes fixed, and connections are converted as a cellspace.
	- Edit State/Transition
	- Place Public Safety Features
		- Click "Place a Public Safety Feature" menu.
		- Choose a feature type (fire extinguisher, indoor hydrant).
		- Click the position to locate a public safety feature.
		- Clicked facet will be highlighted.
		- Click "OK" to finalize the current placement.
	- Interlayer Connection Editor
		- Select a state for a room or corridor.
		- Click "Interlayer Connection Editor" menu.
		- Select states of nonnavigables that belong to the selected room/corridor.
		- Selected states will be highlighted with yellow.
		- Click "OK" button to finalize the connection setting.
	- Export InFactory Queries
		- Export the current state as InFactory queries.
		- In the output file, each query is comprised of two lines:
			- Query URL 
			- Json data
		- You can generate the corresponding IndoorGML file using the included Python script /script/request-infactory.py

## Data file formats 

- Geometry (.geom)
	- If a facet has n vertices without any hole:
		- n 0
		- x1 y1 z1 i1
		- x2 y2 z2 i2
		- ...
		- xn yn zn in
		- (xj,yj,zj) is the coordinate and ij is the index number of vertex j.
	- If a facet has n vertices as its exterior and has h holes.
		- n h
		- xj yj zj ij ( * n lines)
		- h1 (=number of vertices of hole 1)
		- xj yj zj ij ( * h1 lines)
		- so on ...
- Cubemap Texture info (.texinfo)
	- Each cubemap texture information consists of two lines indicating the paths of the texture image file and the camera pose file.
- Obstacle List (.txt)
	- List of pcd file path.
