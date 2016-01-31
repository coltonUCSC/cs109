// $Id: commands.cpp,v 1.16 2016-01-14 16:10:40-08 - - $

#include "commands.h"
#include "debug.h"
#include <stack>

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
}

int exit_status_message() {
   int exit_status = exit_status::get();
   cout << execname() << ": exit(" << exit_status << ")" << endl;
   return exit_status;
}

void fn_cat (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   string filename = *(words.end()-1);
   inode_ptr ogcwd = state.getCwd();
   inode_ptr res = resolvePath(words[1], state.getCwd());
   if (res == nullptr){
      throw command_error ("cat: " + filename + ": file does not exist");
      return; }
   if (res->isDirectory()) return; //error here
   cout << res->getContents()->readfile() << endl;
}

void fn_cd (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   inode_ptr ogcwd = state.getCwd();
   if (words.size() == 1){
      state.setCwd(state.getRoot()->getContents()->getNode("/"));
      return;
   }
   if (words.size() > 2) return;
   if (ogcwd == state.getRoot()->getContents()->getNode("/") && words[1] == "..") return;
   inode_ptr res = resolvePath(words[1], state.getCwd());
   if (res == nullptr) return;
   if (!res->isDirectory()) return;
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
   throw ysh_exit();
}

void fn_ls (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   inode_ptr ogcwd = state.getCwd();
   inode_ptr res = ogcwd;
   if(words.size() > 1)
      res = resolvePath(words[1], state.getCwd());
   if (res == nullptr) return;
   auto pathList = res->getContents()->getAllPaths();
   state.setCwd(res);
   fn_pwd(state, words);
   for (size_t i = 0; i < pathList.size(); i++){
      cout << pathList[i] << endl;
   }
   state.setCwd(ogcwd);
}

void DFS(inode_ptr node) {
   auto dirs = node->getContents()->getAllDirs();
   auto all = node->getContents()->getAllPaths();
   for (auto it = all.begin(); it != all.end(); ++it)
      cout << *it << endl;
   for (auto it = dirs.begin(); it != dirs.end(); ++it){
      DFS(node->getContents()->getNode(*it));
   }
}

void fn_lsr (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   inode_ptr res = resolvePath(words[1], state.getCwd());
   if (res == nullptr) return;
   //fn_ls (state, words);
   DFS(res);
}

void fn_make (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if (words.size() <= 0){
      cout << "mkdir: missing operand" << endl;
      return;
   }
   wordvec newData(words.begin()+2, words.end());

   wordvec pathvec = split (words[1], "/");
   string fullpath = "";
   string filename = *(pathvec.end()-1);
   for (auto it = pathvec.begin(); it != pathvec.end()-1; ++it)
      fullpath += (*it + "/");
   inode_ptr res = resolvePath(fullpath, state.getCwd()); //resulting path before filename
   if (res == nullptr) return;
   inode_ptr file = res->getContents()->getNode(filename); //search directory for filename if existing
   if (file != nullptr && res != nullptr) {
      if(file->isDirectory())
         return;
      file->getContents()->writefile(newData);
      return;
   }
   res->getContents()->mkfile(filename);
   res->getContents()->getNode(filename)->getContents()->writefile(newData);

   inode_ptr ogcwd = state.getCwd();
}

void fn_mkdir (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   if (words.size() <= 0){
      cout << "mkdir: missing operand" << endl;
      return;
   }
   //root case?
   if (words[1] == "/"){
      inode_ptr ogcwd = state.getCwd();
      auto root = state.getCwd()->getContents()->mkdir(words[1]);
      root->getContents()->setPath("..", ogcwd);
      root->getContents()->setPath(".", root);
      return;
   }

   wordvec pathvec = split(words[1], "/");
   string dirname = *(pathvec.end()-1);
   string fullpath = "";
   for (auto it = pathvec.begin(); it != pathvec.end()-1; ++it)
      fullpath += (*it + "/");
   inode_ptr res = resolvePath(fullpath,state.getCwd());
   if (res == nullptr) return;
   inode_ptr directory = res->getContents()->getNode(dirname);
   if (directory != nullptr && res != nullptr) //if filename exists and path exists
      //dont overwrite anything (ie file or directory)
      return;
   inode_ptr dir = res->getContents()->mkdir(dirname);
   dir->getContents()->setPath("..", res);
   dir->getContents()->setPath(".",res->getContents()->getNode(dirname));
}

void fn_prompt (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   state.setPrompt(words[1]);
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
   if (words.size() <= 0) return; //error here?
   wordvec pathvec = split(words[1],"/");
   string fullpath = "";
   string name = *(pathvec.end()-1);
   for (auto it = pathvec.begin(); it != pathvec.end()-1; ++it)
      fullpath += (*it + "/");
   inode_ptr res = resolvePath(fullpath,state.getCwd());
   if (res == nullptr) return; //error
   inode_ptr rmfile = res->getContents()->getNode(name);
   if (res != nullptr && rmfile != nullptr){
      if(rmfile->isDirectory()){
         if(rmfile->getContents()->getAllPaths().size() <= 2){
            res->getContents()->remove(name);
            return;
         } else { return; /* not empty */ }}
         res->getContents()->remove(name);
         return;
   }
}

void fn_rmr (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   inode_ptr ogcwd = state.getCwd();
   inode_ptr res = resolvePath(words[1], state.getCwd());
   if (res == nullptr) return;
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
      //if (!oldcwd->getContents()->getNode(temp[i])->isDirectory())
         //return;
      if (oldcwd == nullptr) return nullptr;
      oldcwd = oldcwd->getContents()->getNode(temp[i]);
   }
   return oldcwd;
}

/*
inode_ptr search (inode_ptr dir, const string& path){
   //inode_ptr = dir->find(path);
   //if !path return;
   //if path return inode_ptr
   //search(inode_ptr)
}
*/
