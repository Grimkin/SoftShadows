#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "Mesh_generated.h"

#include <string>
#include <vector>
#include <iostream>
#include <fstream>

static const std::string fileType = "mesh";

struct Parameters {
	std::string input = "", output = "";
	bool combineMesh = true;
};

void PrintHelp( const std::string& name ) {
	std::cout << "Usage: " << name << " inputfile [-o outputfile] [-c <combineMeshes=true>]" << std::endl;
}

bool ChangeType( std::string& path, const std::string& targetType ) {
	size_t dotPos = path.rfind( '.' );

	bool changed = false;
	if( dotPos != std::string::npos ) {
		if( path.substr( dotPos + 1 ) != fileType )
			changed = true;
		path.erase( path.begin() + dotPos, path.end() );
	}
	path += "." + targetType;

	return changed;
}

bool ReadParameters( int argc, char * argv[], Parameters& params ) {
	if( argc < 2 || std::string( argv[1] ) == "-h" || std::string( argv[1] ) == "--help" ) {
		PrintHelp( argv[0] );
		return false;
	}

	params.input = argv[1];
	
	for( int i = 2; i < argc; i++ ) {
		std::string arg = argv[i];
		if( i + 1 < argc ) {
			if( arg == "-o"  ) {
				params.output = argv[i + 1];
				i++;
			}
			else if( arg == "-c" ) {
				std::string comb = argv[i + 1];
				if( comb == "1" || comb == "true" || comb == "TRUE" || comb == "True" ) {
					params.combineMesh = true;
					i++;
				}
				else if( comb == "0" || comb == "false" || comb == "FALSE" || comb == "False" ) {
					params.combineMesh = false;
					i++;
				}
			}
		}
	}

	if( params.output.empty() ) {
		params.output = params.input;
		ChangeType( params.output, fileType );
	}

	return true;
}

bool ConvertMesh(const std::string& loadPath, bool combineMesh, flatbuffers::FlatBufferBuilder& builder ) {
	using namespace Loader::Mesh;

	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string err;

	std::cout << "Loading '" << loadPath << "'... ";

	bool ret = tinyobj::LoadObj( shapes, materials, err, loadPath.c_str() );
	if( !ret ) {
		std::cout << "failed" << std::endl;
		return false;
	}

	std::cout << "complete" << std::endl;

	std::vector<flatbuffers::Offset<Mesh>> meshes;

	std::cout << "Converting... ";

	if( combineMesh ) {
		std::vector<Vec3> allPositions;
		std::vector<Vec3> allNormals;
		std::vector<uint32_t> allIndices;
		for( size_t i = 0; i < shapes.size(); ++i ) {
			std::vector<Vec3>* positions = reinterpret_cast<std::vector<Vec3>*>( &shapes[i].mesh.positions );
			std::vector<Vec3>* normals = reinterpret_cast<std::vector<Vec3>*>( &shapes[i].mesh.normals );
			std::vector<uint32_t>* indices = &shapes[i].mesh.indices;
			if( allPositions.size() > 0 ) {
				for( uint32_t& val : *indices ) {
					val += static_cast<uint32_t>( allPositions.size() );
				}
			}
			allPositions.insert( allPositions.end(), positions->begin(), positions->end() );
			allNormals.insert( allNormals.end(), normals->begin(), normals->end() );
			allIndices.insert( allIndices.end(), indices->begin(), indices->end() );
		}
		auto posVec = builder.CreateVectorOfStructs( allPositions );
		auto normalVec = builder.CreateVectorOfStructs( allNormals );
		auto indicesVec = builder.CreateVector( allIndices );
		auto mesh = CreateMesh( builder, posVec, normalVec, indicesVec );
		meshes.push_back( mesh );
	}
	else {
		for( size_t i = 0; i < shapes.size(); ++i ) {
			std::vector<Vec3>* positions = reinterpret_cast<std::vector<Vec3>*>( &shapes[i].mesh.positions );
			std::vector<Vec3>* normals = reinterpret_cast<std::vector<Vec3>*>( &shapes[i].mesh.normals );
			auto posVec = builder.CreateVectorOfStructs( *positions );
			auto normalVec = builder.CreateVectorOfStructs( *normals );
			auto indicesVec = builder.CreateVector( shapes[i].mesh.indices );
			auto mesh = CreateMesh( builder, posVec, normalVec, indicesVec );
			meshes.push_back( mesh );
		}
	}

	auto meshesVec = builder.CreateVector( meshes );
	auto geometries = CreateMeshes( builder, meshesVec );

	Loader::Mesh::FinishMeshesBuffer( builder, geometries );

	std::cout << "complete" << std::endl;

	return true;
}

bool SaveMesh( std::string savePath, flatbuffers::FlatBufferBuilder& builder ) {
	size_t dotPos = savePath.rfind( '.' );

	if( ChangeType( savePath, fileType ) )
		std::cout << "Changed output type to '." << fileType << "'" << std::endl;
	
	std::cout << "Saving to '" << savePath << "'... ";

	std::ofstream file( savePath, std::ios::out | std::ios::trunc | std::ios::binary );
	if( !file.is_open() ) {
		std::cout << "failed" << std::endl;
		return false;
	}

	file.write( reinterpret_cast<char*>( builder.GetBufferPointer() ), builder.GetSize() );

	if( file.bad() ) {
		std::cout << "failed" << std::endl;
		return false;
	}
	
	std::cout << "complete" << std::endl;

	return true;
}

void main( int argc, char *argv[] ) {
	Parameters params;
	if( !ReadParameters( argc, argv, params ) )
		return;

	flatbuffers::FlatBufferBuilder builder;
	if( !ConvertMesh( params.input, params.combineMesh, builder ) ) 
		return;

	SaveMesh( params.output, builder );
}
