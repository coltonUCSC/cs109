// $Id: commands.cpp,v 1.16 2016-01-14 16:10:40-08 - - $

#include "commands.h"
#include "debug.h"

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
   inode_ptr ogcwd = state.getCwd();
   inode_ptr res = resolvePath(words[1], state.getCwd());
   if (res == nullptr) return;
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
   if (res->isDirectory()) return;
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
   for (size_t i = 0; i < pathList.size(); i++){
      cout << pathList[i] << endl;
   }
}

void fn_lsr (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

void fn_make (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if (words.size() <= 0){
      cout << "mkdir: missing operand" << endl;
      return;
   }
   wordvec newData(words.begin()+2, words.end());
   if (state.getCwd()->getContents()->getNode(words[1]) != nullptr){
      if (!state.getCwd()->getContents()->getNode(words[1])->isDirectory()){
         state.getCwd()->getContents()->getNode(words[1])->getContents()->writefile(newData);
         return;
      } else { return; /* error here */ }
   }
   inode_ptr newFile = state.getCwd()->getContents()->mkfile(words[1]);
   //wordvec newData(words.begin()+2, words.end());
   newFile->getContents()->writefile(newData);
}

void fn_mkdir (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   if (words.size() <= 0){
      cout << "mkdir: missing operand" << endl;
      return;
   }
   inode_ptr ogcwd = state.getCwd();
   for (auto it = words.begin()+1; it != words.end(); ++it){
      auto newDir = state.getCwd()->getContents()->mkdir(*it);
      newDir->getContents()->setPath("..", ogcwd);
      newDir->getContents()->setPath(".", newDir);
   }
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
   for (auto it = words.begin()+1; it != words.end(); ++it){
      state.getCwd()->getContents()->remove(*it);
   }
}

void fn_rmr (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

inode_ptr resolvePath (const string& path, inode_ptr oldcwd){
   wordvec temp = split (path, "/");
   for(unsigned i=0; i < temp.size(); i++){
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
