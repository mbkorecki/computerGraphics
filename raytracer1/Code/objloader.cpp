#include "objloader.h"

// Pro C++ Tip: here you can specify other includes you may need
// such as <iostream>

#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;

// ===================================================================
// -- Constructors and destructor ------------------------------------
// ===================================================================

// --- Public --------------------------------------------------------

OBJLoader::OBJLoader(string const &filename)
	:
	d_hasTexCoords(false)
{
	parseFile(filename);
}

// ===================================================================
// -- Member functions -----------------------------------------------
// ===================================================================

// --- Public --------------------------------------------------------

vector<Vertex> OBJLoader::vertex_data() const
{
	vector<Vertex> data;

	// For all vertices in the model, interleave the data
	for (Vertex_idx const &vertex : d_vertices)
	{
		// Add coordinate data
		Vertex vert;

		vec3 const coord = d_coordinates.at(vertex.d_coord);
		vert.x = coord.x;
		vert.y = coord.y;
		vert.z = coord.z;

		// Add normal data
		vec3 const norm = d_normals.at(vertex.d_norm);
		vert.nx = norm.x;
		vert.ny = norm.y;
		vert.nz = norm.z;

		// Add texture data (if available)
		if (d_hasTexCoords)
		{
			vec2 const tex = d_texCoords.at(vertex.d_tex);
			vert.u = tex.u; // u coordinate
			vert.v = tex.v; // v coordinate
		} else {
			vert.u = 0;
			vert.v = 0;
		}
		data.push_back(vert);
	}

	return data; // copy elision
}

unsigned OBJLoader::numTriangles() const
{
	return d_vertices.size() / 3U;
}

bool OBJLoader::hasTexCoords() const
{
	return d_hasTexCoords;
}

vector<Vertex> OBJLoader::unitize()
{
	// TODO: implement this yourself!

	// This is a very handy function for importing models
	// which you may reuse in other projects.
	// You may have noticed you can use arbitrary sizes for your
	// models. You may find that modelers do not always use the
	// same size for models. Therefore it might be preferable to
	// scale the object to fit inside the unit cube so you can easily
	// set the right scale of your model in OpenGL. Aditionally,
	// the model does not have to be centered around the origin
	// (0, 0, 0) which may cause troubles when translating
	// This function should fix that!

	// A common approach looks like this:

	// Determine min / max and offset in each dimension
	// Determine by which factor to scale (largest difference
	//  in min / max in a dimension (Important! Scale uniformaly in
	//  all dimensions!)
	// Loop over all coordinate data and scale the coordinates
	//  and apply the translate/scaling

	vector<Vertex> vertices = vertex_data();

	int size = 100;
	int offset_x = 200;
	int offset_y = 200;
	int offset_z = 200;

	float min_x = 0;
	float max_x = 0;
	float min_y = 0;
	float max_y = 0;
	float min_z = 0;
	float max_z = 0;
	for (uint i = 0; i < vertices.size(); i++)
	{
		if (vertices[i].x < min_x)
			min_x = vertices[i].x;
		if (vertices[i].x > max_x)
			max_x = vertices[i].x;
		if (vertices[i].y < min_y)
			min_y = vertices[i].y;
		if (vertices[i].y > max_y)
			max_y = vertices[i].y;
		if (vertices[i].z < min_z)
			min_z = vertices[i].z;
		if (vertices[i].z > max_z)
			max_z = vertices[i].z;
	}

	float center_x = (min_x + max_x) / 2;
	float center_y = (min_y + max_y) / 2;
	float center_z = (min_z + max_z) / 2;

	float mag_x = (max_x - min_x) / 2;
	float mag_y = (max_y - min_y) / 2;
	float mag_z = (max_z - min_z) / 2;

	for (uint i = 0; i < vertices.size(); i++)
	{
		Vertex new_vertex;
		new_vertex = vertices[i];

		new_vertex.x = (new_vertex.x - center_x) / mag_x * size + offset_x;
		new_vertex.y = (new_vertex.y - center_y) / mag_y * size + offset_y;
		new_vertex.z = (new_vertex.z - center_z) / mag_z * size + offset_z;

		vertices[i] = new_vertex;
	}

	return vertices;
}

// --- Private -------------------------------------------------------

void OBJLoader::parseFile(string const &filename)
{
	ifstream file(filename);
	if (file)
	{
		string line;
		while(getline(file, line))
			parseLine(line);

	} else {
		cerr << "Could not open: " << filename << " for reading!\n";
	}
}

void OBJLoader::parseLine(string const &line)
{
	if (line[0] == '#')
		return;             // ignore comments

	StringList tokens = split(line, ' ', false);

	if (tokens[0] == "v")
		parseVertex(tokens);
	else if (tokens[0] == "vn")
		parseNormal(tokens);
	else if (tokens[0] == "vt")
		parseTexCoord(tokens);
	else if (tokens[0] == "f")
		parseFace(tokens);

	// Other data is also ignored
}

void OBJLoader::parseVertex(StringList const &tokens)
{
	float x, y, z;
	x = stof(tokens.at(1));     // 0 is the "v" token
	y = stof(tokens.at(2));
	z = stof(tokens.at(3));
	d_coordinates.push_back(vec3{x, y, z});
}

void OBJLoader::parseNormal(StringList const &tokens)
{
	float x, y, z;
	x = stof(tokens.at(1));     // 0 is the "vn" token
	y = stof(tokens.at(2));
	z = stof(tokens.at(3));
	d_normals.push_back(vec3{x, y, z});
}

void OBJLoader::parseTexCoord(StringList const &tokens)
{
	d_hasTexCoords = true;      // Texture data will be read

	float u, v;
	u = stof(tokens.at(1));     // 0 is the "vt" token
	v = stof(tokens.at(2));
	d_texCoords.push_back(vec2{u, v});
}

void OBJLoader::parseFace(StringList const &tokens)
{
	// skip the first token ("f")
	for (size_t idx = 1; idx < tokens.size(); ++idx)
	{
		// format is:
		// <vertex idx + 1>/<texture idx +1>/<normal idx + 1>
		// Wavefront .obj files start counting from 1 (yuck)

		StringList elements = split(tokens.at(idx), '/');
		Vertex_idx vertex {}; // initialize to zeros on all fields

		vertex.d_coord = stoul(elements.at(0)) - 1U;

		if (d_hasTexCoords)
			vertex.d_tex = stoul(elements.at(1)) - 1U;
		else
			vertex.d_tex = 0U; // ignored

		vertex.d_norm = stoul(elements.at(2)) - 1U;

		d_vertices.push_back(vertex);
	}
}

OBJLoader::StringList OBJLoader::split(string const &line,
                                       char splitChar,
                                       bool keepEmpty)
{
	StringList tokens;
	istringstream iss(line);
	string token;
	while (getline(iss, token, splitChar))
		if (token.size() > 0 || (token.size() == 0 && keepEmpty))
			tokens.push_back(token);

	return tokens;
}
