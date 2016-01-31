// $Id: file_sys.cpp,v 1.5 2016-01-14 16:16:52-08 - - $

#include <iostream>
#include <stdexcept>
#include <unordered_map>

using namespace std;

#include "debug.h"
#include "file_sys.h"

int inode::next_inode_nr {1};

struct file_type_hash {
   size_t operator() (file_type type) const {
      return static_cast<size_t> (type);
   }
};

ostream& operator<< (ostream& out, file_type type) {
   static unordered_map<file_type,string,file_type_hash> hash {
      {file_type::PLAIN_TYPE, "PLAIN_TYPE"},
      {file_type::DIRECTORY_TYPE, "DIRECTORY_TYPE"},
   };
   return out << hash[type];
}

inode_state::inode_state() {
   DEBUGF ('i', "root = " << root << ", cwd = " << cwd
          << ", prompt = \"" << prompt() << "\"");
   inode_ptr newNode = make_shared<inode>(file_type::DIRECTORY_TYPE);
   root = newNode;
   cwd = root;
}

const string& inode_state::prompt() { return prompt_; }

void inode_state::setPrompt(string p) { prompt_ = p; }

inode_ptr inode_state::getCwd(){
  return cwd;
}

void inode_state::setCwd(inode_ptr node){
  cwd = node;
}

// TODO double check to make sure this does not
// violate encapsulation rules
inode_ptr inode_state::getRoot(){
  return root;
}

ostream& operator<< (ostream& out, const inode_state& state) {
   out << "inode_state: root = " << state.root
       << ", cwd = " << state.cwd;
   return out;
}

inode::inode(file_type type): inode_nr (next_inode_nr++) {
   switch (type) {
      case file_type::PLAIN_TYPE:
           contents = make_shared<plain_file>();
           isDir = false;
           break;
      case file_type::DIRECTORY_TYPE:
           contents = make_shared<directory>();
           isDir = true;
           break;
   }
   DEBUGF ('i', "inode " << inode_nr << ", type = " << type);
}

int inode::get_inode_nr() const {
   DEBUGF ('i', "inode = " << inode_nr);
   return inode_nr;
}

base_file_ptr inode::getContents(){
  return contents;
}

file_error::file_error (const string& what):
            runtime_error (what) {
}

size_t plain_file::size() const {
   size_t size {0};
   DEBUGF ('i', "size = " << size);
   return size;
}

const wordvec& plain_file::readfile() const {
   DEBUGF ('i', data);
   return data;
}

void plain_file::writefile (const wordvec& words) {
   DEBUGF ('i', words);
   data = words;
}

void plain_file::remove (const string&) {
   throw file_error ("is a plain file");
}

inode_ptr plain_file::mkdir (const string&) {
   throw file_error ("is a plain file");
}

inode_ptr plain_file::mkfile (const string&) {
   throw file_error ("is a plain file");
}

void plain_file::setPath(const string&, inode_ptr){
   throw file_error ("is a plain file");
}

string plain_file::getPath(inode_ptr){
   throw file_error ("is a plain file");
}

wordvec plain_file::getAllPaths(){
  throw file_error ("is a plain file");
}

wordvec plain_file::getAllDirs(){
  throw file_error ("is a plain file");
}

inode_ptr plain_file::getNode(const string&){
  throw file_error ("is a plain file");
}

void plain_file::printMap(){
  throw file_error ("is a plain file");
}

wordvec plain_file::getAllFiles(){
   throw file_error ("nah");
}
string plain_file::getPwd(){
  throw file_error ("is a plain file");
}

void plain_file::setPwd(string){
  throw file_error ("is a plain file");
}

size_t directory::size() const {
   size_t size {0};
   DEBUGF ('i', "size = " << size);
   return size;
}

const wordvec& directory::readfile() const {
   throw file_error ("is a directory");
}

void directory::writefile (const wordvec&) {
   throw file_error ("is a directory");
}

// Preconditions:
// filename is the relative pathname of the parent
// directory of the file or directory to remove.
// If the filename maps to a directory, there will be no
// subdirectories (or files?) inside.  Any recursion logic
// will be handled by fn_rmr()
void directory::remove (const string& filename) {
   DEBUGF ('i', filename);
   dirents.erase(filename);
}

inode_ptr directory::mkdir (const string& dirname) {
   DEBUGF ('i', dirname);
   inode_ptr newDir = make_shared<inode>(file_type::DIRECTORY_TYPE);
   dirents.insert(pair<string,inode_ptr>(dirname, newDir));
   newDir->getContents()->setPwd(fullPath + "/" + dirname);
   return newDir;
}

inode_ptr directory::mkfile (const string& filename) {
   DEBUGF ('i', filename);
   inode_ptr newFile = make_shared<inode>(file_type::PLAIN_TYPE);
   dirents.insert(pair<string,inode_ptr>(filename, newFile));
   return newFile;
}

void directory::setPath(const string& name, inode_ptr node){
  dirents.insert(pair<string,inode_ptr>(name,node));
}

string directory::getPath(inode_ptr node){
  for (auto iter = dirents.begin(); iter != dirents.end(); ++iter){
    if (iter->second == node){
      return iter->first;
    }
  }
  return nullptr;
}

wordvec directory::getAllPaths(){
  wordvec pathList;
  for (auto iter = dirents.begin(); iter != dirents.end(); ++iter){
    pathList.push_back(iter->first);
  }
  return pathList;
}

wordvec directory::getAllDirs(){
  wordvec dirList;
  for (auto iter = dirents.begin(); iter != dirents.end(); ++iter){
    if ((iter->second->isDirectory()) && (iter->first != ".") && (iter->first != ".."))
      dirList.push_back(iter->first);
  }
  return dirList;
}

wordvec directory::getAllFiles(){
   wordvec fileList;
   for (auto iter = dirents.begin(); iter != dirents.end(); ++iter){
      if (iter->first != "." && iter->first != ".."){
         fileList.push_back(iter->first);
         cout << iter->first << endl;
      }

   }
   return fileList;
}

inode_ptr directory::getNode(const string& path){
   auto it = dirents.find(path);
   if (it == dirents.end())
      return nullptr;
   return dirents.find(path)->second;
}

void directory::printMap(){
  cout << "Map contents:" << endl;
  for (auto it = dirents.begin(); it != dirents.end(); ++it){
    cout << it->first << " -> " << it->second << endl;
  }
  cout << endl;
}

string directory::getPwd(){
  return fullPath;
}

void directory::setPwd(string newPwd){
  fullPath = newPwd;
}
