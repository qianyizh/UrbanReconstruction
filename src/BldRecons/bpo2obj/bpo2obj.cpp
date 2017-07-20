//-----------------------------------------------------------------------------
// bpo2obj.cpp : Converts bpo binary files into OBJ files.
// Author: Qianyi Zhou.
//-----------------------------------------------------------------------------

#include "stdafx.h"

#include "graphics/obj_writer.h"

#include "Geometry/Outline.h"
#include "Miscs/BPOReader.h"

#include "utils/version_info.h"
#include "utils/console.h"

#include <string>
#include <vector>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

using graphics::Obj;
using graphics::IndicesSet;
using graphics::Indices;
using graphics::Coordinates;
using graphics::ObjWriter;

using utils::magenta;
using utils::cyan;
using utils::red;
using utils::defaults;

using std::cout;
using std::endl;
using std::vector;
using std::string;
using std::exception;

namespace po = boost::program_options;
namespace fs = boost::filesystem;
using boost::filesystem::directory_iterator;

int main(int argc, char* argv[])
{
    string output_basename;
    bool verbose = false;

	po::options_description desc("Options");
    desc.add_options()
		("output-basename,o", po::value<string>(&output_basename)->default_value("outlines_"), "Output basename.")
        ("verbose,v", "Verbose mode.")
		("help,h", "Print help.")
		("color", po::value<string>()->implicit_value("auto"), "Colorize output. Value must be auto (the default), never, or always.")
		("version", "Print version.");

    po::positional_options_description p;
    p.add("cmd-files-dirs", -1);

    vector<string> cmd_files_dirs;
    po::options_description hidden("Hidden options");
    hidden.add_options()
        ("cmd-files-dirs", po::value< vector<string> >(&cmd_files_dirs)->composing(), "")
        ;

    po::options_description cmdline_options;
    cmdline_options.add(desc).add(hidden);

    po::variables_map vm;
    try
    {
		po::store(po::command_line_parser(argc, argv).options(cmdline_options).positional(p).run(), vm);
		if (fs::exists("bpo2obj.cfg"))
	    {
			cout << "Loading bpo2obj.cfg" << endl;
	    	po::store(po::parse_config_file<char>("bpo2obj.cfg", cmdline_options), vm);
	    }
		po::notify(vm);
    }
	catch (exception& e)
	{
		cout << e.what() << endl;
		return 1;
	}

	//-----------------------------------------------------------------------------

	try
	{
		if (vm.count("color"))
		{
			string mode = vm["color"].as<string>();
			if (mode == "auto")
			{
				utils::Console::auto_color();
			}
			else if (mode == "always")
			{
				utils::Console::use_color = true;
			}
			else if (mode == "never")
			{
				utils::Console::use_color = false;
			}
			else
			{
				cout << "Invalid argument to --color: should be auto|never|always." << endl;
				return 0;
			}
		}
		else
		{
			utils::Console::auto_color();
		}

		if (vm.count("help"))
		{
			cout << "`bpo2obj` converts building outline files in BPO format to OBJ.\n\n"
					"Usage: bpo2obj [files|dirs] [options]\n\n"
				 << desc << endl
				 << "bpo2obj.cfg is parsed for options if present." << endl;
			return 0;
		}

		if (vm.count("version"))
		{
			cout << VERSION_INFO << endl;
			return 0;
		}

		if (vm.count("verbose"))
		{
			verbose = true;
		}

		if (vm.count("cmd-files-dirs"))
		{
			cmd_files_dirs = vm["cmd-files-dirs"].as< vector<string> >();
		}
		else
		{
			cout << "cmd-files-dirs not specified." << endl;
			return 1;
		}

		if (vm.count("output-basename"))
		{
			output_basename = vm["output-basename"].as<string>();
			if (!output_basename.empty())
			{
				fs::path dir = fs::path(output_basename).remove_leaf();
				if (!dir.empty())
				{
					if (!fs::exists(dir))
					{
						cout << "No such directory " << dir << "." << endl;
						return 1;
					}
				}
			}
		}

		//-----------------------------------------------------------------------------

		if (verbose)
		{
			for (vector<string>::const_iterator itr = cmd_files_dirs.begin(); itr != cmd_files_dirs.end(); ++itr)
			{
				cout << "cmd-files-dirs=" << *itr << endl;
			}
			cout << "output-basename=" << output_basename << endl;
		}

		//-----------------------------------------------------------------------------

		vector<string> obj_filenames;
		for (vector<string>::const_iterator itr = cmd_files_dirs.begin(); itr != cmd_files_dirs.end(); itr++)
		{
			if (!fs::exists(*itr))
			{
				cout << red << "Error: No such file or directory " << *itr << defaults << endl;
				return 1;
			}

			if (fs::is_directory(*itr))
			{
				directory_iterator end_itr;
				for (directory_iterator ditr(*itr); ditr != end_itr; ++ditr)
				{
					if (ditr->path().extension() != ".bpo")
					{
						continue;
					}

					obj_filenames.push_back(ditr->path().string());
				}
			}
			else
			{
				obj_filenames.push_back(*itr);
			}
		}

		//-----------------------------------------------------------------------------

		ObjWriter obj_writer;

		for (vector<string>::const_iterator itr = obj_filenames.begin(); itr != obj_filenames.end(); itr++)
		{
			if (verbose)
			{
				cout << "Processing " << *itr << endl;
			}

			CBPOReader reader;
			reader.OpenFile(itr->c_str());

			BPOHeader header = reader.ReadHeader();

			Obj obj;
			Coordinates& vertices = obj.vertices();
			unsigned int vertex_count = 0;
			
			for (int i = 0; i < header.number; i++)
			{
				COutline outline;
				reader.ReadOutline(outline);

				const vector<COutlineVertex>& outline_verts = outline.m_vecVertex;
				vertices.push_back(outline_verts[0].v.pVec[0]);
				vertices.push_back(outline_verts[0].v.pVec[1]);
				vertices.push_back(outline_verts[0].v.pVec[2]);
				
				Indices indices;
				indices.push_back(vertex_count++);

				int idx = 0;
				int start_index = 0;
				while (start_index != outline_verts[idx].next)
				{
					idx = outline_verts[idx].next;
					const CVector3D& vec = outline_verts[idx].v;
					vertices.push_back(vec.pVec[0]);
					vertices.push_back(vec.pVec[1]);
					vertices.push_back(vec.pVec[2]);
					indices.push_back(vertex_count++);
				}

				obj.line_indices().push_back(indices);
			}

			reader.CloseFile();

			if (obj.vertices().empty())
			{
				if (verbose)
				{
					cout << magenta << "Warning: skipping empty outline." << defaults << endl;
				}
				continue;
			}

			string output_file_name = output_basename + fs::path(*itr).replace_extension(".obj").leaf().string();
			if (verbose)
			{
				cout << "Writing " << output_file_name << endl;
			}
			obj_writer.Write(output_file_name, obj);
		}
	}
	catch (exception const& e)
	{
		cout << e.what() << endl;
		return 1;
	}
	return 0;
}
