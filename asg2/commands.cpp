// $Id: commands.cpp,v 1.16 2016-01-14 16:10:40-08 - - $

#include "commands.h"
#include "debug.h"
#include <stack>
#include <iomanip>

command_hash cmd_hash {
	{"cat"   , fn_cat   },
	{"cd"    , fn_cd    },
	{"echo"  , fn_echo  },
	{"exit"  , fn_exit  },
	{"ls"    , fn_ls    },
	{"lsr"   , fn_lsr   },
	{"make"  , fn_make  },
	{"mkdir" , fn_mkdir },
	{"prompt", fn_prompt},
	{"pwd"   , fn_pwd   },
	{"rm"    , fn_rm    },
	{"rmr"   , fn_rmr   },
};

command_fn find_command_fn (const string& cmd) {
   // Note: value_type is pair<const key_type, mapped_type>
   // So: iterator->first is key_type (string)
   // So: iterator->second is mapped_type (command_fn)
	const auto result = cmd_hash.find (cmd);
	if (result == cmd_hash.end()) {
		throw command_error (cmd + ": no such function");
	}
	return result->second;
}

command_error::command_error (const string& what):
runtime_error (what) {
	exit_status::set(1);
}

int exit_status_message() {
	int exit_status = exit_status::get();
	cout << execname() << ": exit(" << exit_status << ")" << endl;
	return exit_status;
}

void fn_cat (inode_state& state, const wordvec& words){
	DEBUGF ('c', state);
	DEBUGF ('c', words);
	if (words.size() <= 1){
		throw command_error ("cat: missing operands"); 
		return;
	}
	for (auto it = words.begin()+1; it != words.end(); ++it){
		string filename = *it;
		inode_ptr res;
		if ((*it)[0] == '/')
			res = resolvePath(*it, state.getRoot()->getContents()->getNode("/"));
		else 
			res = resolvePath(*it, state.getCwd());
		if (res == nullptr){
			throw command_error ("cat: " + filename + ": No such file or directory");
			return; 
		}
   		if (res->isDirectory()){
   			throw command_error ("cat: " + filename + ": Is a directory");
   			return;
   		} 
   		cout << res->getContents()->readfile() << endl;
	}
}

void fn_cd (inode_state& state, const wordvec& words){
	DEBUGF ('c', state);
	DEBUGF ('c', words);
	if (words.size() > 2){
		throw command_error ("cd: Only one operand may be given"); 
		return;
	}
	if (words.size() == 1 || words[1] == "/"){
		state.setCwd(state.getRoot()->getContents()->getNode("/"));
		return;
	}
	
	inode_ptr res = resolvePath(words[1], state.getCwd());
	if (res == nullptr) {
		throw command_error ("cd: " + words[1] + ": No such file or directory");
	}
	if (!res->isDirectory()){ 
		throw command_error ("cd: " + words[1] + ": Is not a directory");
		return;
	}
	state.setCwd(res);
}

void fn_echo (inode_state& state, const wordvec& words){
	DEBUGF ('c', state);
	DEBUGF ('c', words);
	cout << word_range (words.cbegin() + 1, words.cend()) << endl;
}

void fn_exit (inode_state& state, const wordvec& words){
	DEBUGF ('c', state);
	DEBUGF ('c', words);
	if (words.size() == 1){
		exit_status::set(0);
	}
	else if (isdigit(words[1][0])){
		exit_status::set(atoi(words[1].c_str()));
	}
	else {
		exit_status::set(127);
	}
	throw ysh_exit();
}

void fn_ls (inode_state& state, const wordvec& words){
	DEBUGF ('c', state);
	DEBUGF ('c', words);
	inode_ptr ogcwd = state.getCwd();

	for (size_t i=0; i < words.size(); i++){
		inode_ptr res = ogcwd;
		if (words.size() > 1 && i == 0)
			continue;
		if (words.size() > 1){
			if (words[i][0] == '/')
				res = resolvePath(words[i], state.getRoot()->getContents()->getNode("/"));
			else 
				res = resolvePath(words[i], state.getCwd());
		}
		if (words[i] == "/")
			res = state.getRoot()->getContents()->getNode("/");
		if (res == nullptr) {
			throw command_error ("ls: " + words[i] + ": No such file or directory");
		}
		auto pathList = res->getContents()->getAllPaths();
		state.setCwd(res);
		string pwd = res->getContents()->getPwd();
		if (words.size() == 1){
			if (pwd.length() == 2){
				cout << "/:" << endl;
			}else{
				pwd = pwd.substr(2,pwd.length()-2);
				cout << pwd << ":" << endl;
			} 
		} else {
			cout << words[i] << ":" << endl;
		}
		for (size_t i = 0; i < pathList.size(); i++){
			if (res->getContents()->getNode(pathList[i])->isDirectory() && (pathList[i] != "..") && (pathList[i] != "."))
				cout << setw(6) << res->getContents()->getNode(pathList[i])->get_inode_nr()-1 << setw(6)
			<< res->getContents()->getNode(pathList[i])->getContents()->getsize() << " " << pathList[i] << "/" << endl;
			else
				cout << setw(6) << res->getContents()->getNode(pathList[i])->get_inode_nr()-1 << setw(6)
			<< res->getContents()->getNode(pathList[i])->getContents()->getsize() << " "  << pathList[i] << endl;
		}
		state.setCwd(ogcwd);
	}
}

void DFS(inode_ptr node) {
	auto dirs = node->getContents()->getAllDirs();
	auto all = node->getContents()->getAllPaths();
	string pwd = node->getContents()->getPwd();
	if (pwd.length() == 2){
		cout << "/:" << endl;
	}
	else{ 
		pwd = pwd.substr(2, pwd.length()-2);
		cout << pwd << ":" << endl;
	}
	for (auto it = all.begin(); it != all.end(); ++it){
		if (node->getContents()->getNode(*it)->isDirectory() && (*it != "..") && (*it != "."))
			cout << setw(6) << node->getContents()->getNode(*it)->get_inode_nr()-1 << setw(6)
		<< node->getContents()->getNode(*it)->getContents()->getsize() << " " << *it << "/" << endl;
		else 
			cout << setw(6) << node->getContents()->getNode(*it)->get_inode_nr()-1 << setw(6)
		<< node->getContents()->getNode(*it)->getContents()->getsize() << " "  << *it << endl; 
	}
	for (auto it = dirs.begin(); it != dirs.end(); ++it){
		DFS(node->getContents()->getNode(*it));
	}
}

void fn_lsr (inode_state& state, const wordvec& words){
	DEBUGF ('c', state);
	DEBUGF ('c', words);
	if (words.size() == 1){
		DFS(state.getCwd());
		return;
	}
	for(auto it = words.begin()+1; it != words.end(); ++it){
		inode_ptr res;
		if (*it == "/")
			res = state.getRoot()->getContents()->getNode("/");
		else
			if ((*it)[0] == '/')
				res = resolvePath(*it, state.getRoot()->getContents()->getNode("/"));
			else
				res = resolvePath(*it, state.getCwd());
		if (res == nullptr) {
			throw command_error ("lsr: " + *it + ": No such file or directory");
		}
		DFS(res);
	}
}

void fn_make (inode_state& state, const wordvec& words){
	DEBUGF ('c', state);
	DEBUGF ('c', words);
	if (words.size() <= 1){
		throw command_error ("make: Missing operands");
		return;
	}
	wordvec newData(words.begin()+2, words.end());
	wordvec pathvec = split (words[1], "/");
	string fullpath = "";
	string filename = *(pathvec.end()-1);
	for (auto it = pathvec.begin(); it != pathvec.end()-1; ++it)
		fullpath += (*it + "/");
    inode_ptr res;
    if (fullpath[0] == '/')
    	res = resolvePath(fullpath, state.getRoot()->getContents()->getNode("/"));
   	else
   		res = resolvePath(fullpath, state.getCwd()); //resulting path before filename
   if (res == nullptr) {
		throw command_error ("make: " + fullpath + ": No such file or directory");
		return;
	}
   inode_ptr file = res->getContents()->getNode(filename); //search directory for filename if existing
   if (file != nullptr && res != nullptr) {
   	if(file->isDirectory()) {
   		throw command_error ("make: " + fullpath + ": Is a directory");
   		return;
   	}
   	file->getContents()->writefile(newData);
   	return;
   }
   res->getContents()->mkfile(filename);
   res->getContents()->getNode(filename)->getContents()->writefile(newData);
}

void fn_mkdir (inode_state& state, const wordvec& words){
	DEBUGF ('c', state);
	DEBUGF ('c', words);

	if (words.size() <= 1){
		throw command_error ("mkdir: missing operand");
		return;
	}

	if (words[1] == "/"){
		auto root = state.getCwd()->getContents()->mkdir(words[1]);
		root->getContents()->setPath("..", state.getCwd());
		root->getContents()->setPath(".", root);
		return;
	}

	wordvec pathvec = split(words[1], "/");
	string dirname = *(pathvec.end()-1);
	string fullpath = "";
	for (auto it = pathvec.begin(); it != pathvec.end()-1; ++it)
		fullpath += (*it + "/");
	inode_ptr res;
    if (fullpath[0] == '/')
    	res = resolvePath(fullpath, state.getRoot()->getContents()->getNode("/"));
   	else
   		res = resolvePath(fullpath, state.getCwd());
	if (res == nullptr) {
		throw command_error ("mkdir: " + fullpath + ": No such file or directory");
		return;
	}
	inode_ptr directory = res->getContents()->getNode(dirname);
    if (directory != nullptr && res != nullptr){ //if filename exists and path exists
        //dont overwrite anything (ie file or directory)
   		throw command_error ("mkdir: " + fullpath + ": Directory already exists");
   		return;
   	}
   inode_ptr dir = res->getContents()->mkdir(dirname);
   dir->getContents()->setPath("..", res);
   dir->getContents()->setPath(".",res->getContents()->getNode(dirname));
}

void fn_prompt (inode_state& state, const wordvec& words){
	DEBUGF ('c', state);
	DEBUGF ('c', words);
	if (words.size() <= 1){
		throw command_error ("prompt: missing operand");
		return;
	}
	string newP;
	for (auto it = words.begin()+1; it != words.end(); ++it){
		newP += *it;
		newP += " ";
	}
	state.setPrompt(newP);
}

void fn_pwd (inode_state& state, const wordvec& words){
	DEBUGF ('c', state);
	DEBUGF ('c', words);
	string pwd = state.getCwd()->getContents()->getPwd();
	if (pwd.length() == 2){
		cout << "/" << endl;
	} else {
		pwd = pwd.substr(2, pwd.length()-2);
		cout << pwd << endl;
	}
}

void fn_rm (inode_state& state, const wordvec& words){
	DEBUGF ('c', state);
	DEBUGF ('c', words);
	if (words.size() <= 1){
		throw command_error ("rm: missing operands");
		return;
	} 
	wordvec pathvec = split(words[1],"/");
	string fullpath = "";
	string name = *(pathvec.end()-1);
	for (auto it = pathvec.begin(); it != pathvec.end()-1; ++it)
		fullpath += (*it + "/");
	inode_ptr res;
    if (fullpath[0] == '/')
    	res = resolvePath(fullpath, state.getRoot()->getContents()->getNode("/"));
   	else
   		res = resolvePath(fullpath, state.getCwd());
	if (res == nullptr) {
		throw command_error ("rm: " + fullpath + ": No such file or directory");
		return;
	}
	inode_ptr rmfile = res->getContents()->getNode(name);
	if (res != nullptr && rmfile != nullptr){
		if(rmfile->isDirectory()){
			if(rmfile->getContents()->getAllPaths().size() <= 2){
				res->getContents()->remove(name);
				return;
     		} else {
     			throw command_error ("rm: " + fullpath + ": Is a directory that is not empty"); 
     			return;
     		}
     	}
     	res->getContents()->remove(name);
     	return;
     }
 }

void fn_rmr (inode_state& state, const wordvec& words){
 	DEBUGF ('c', state);
 	DEBUGF ('c', words);
 	if (words.size() <= 1){
 		throw command_error ("rmr: missing operands");
 		return;
 	}
 	inode_ptr res;
    if (words[1][0] == '/')
    	res = resolvePath(words[1], state.getRoot()->getContents()->getNode("/"));
   	else
   		res = resolvePath(words[1], state.getCwd());
 	if (res == nullptr) {
 		throw command_error ("rmr: " + words[1] + ": No such file or directory");
 		return;
 	}
 	DFSr(res);
 	fn_rm(state, words);
 }

//recursive call, call until the deepest directoy has no more directories
//begin deleting its contents, function returns and is then continued from
//its parents call, begin deleting its children and so on...
//sudo depth?
void DFSr(inode_ptr node){
   	auto dirs = node->getContents()->getAllDirs();
   	auto files = node->getContents()->getAllFiles();
   	for (auto it = dirs.begin(); it != dirs.end(); ++it){
   		DFSr(node->getContents()->getNode(*it));
   	}
   	for (auto it = files.begin(); it != files.end(); ++it){
   		node->getContents()->remove(*it);
   	}
}

inode_ptr resolvePath (const string& path, inode_ptr oldcwd){
   	wordvec temp = split (path, "/");
	for(unsigned i=0; i < temp.size(); i++){
   		if (oldcwd == nullptr) return nullptr;
   		oldcwd = oldcwd->getContents()->getNode(temp[i]);
    }
   	return oldcwd;
}
