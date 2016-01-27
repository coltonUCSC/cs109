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

// TODO modify all functions that can take either relative or
// full pathnames, if pathname is full (starts with /) then we 
// need to create and call a helper function that would traverse
// the tree by searching each directory map for the partial string.
// eg cd /home/colton/dir would need to set cwd to /, then search
// the directory map for the next node (home).
void fn_cat (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   // We should really consider breaking this up.
   cout << state.getCwd()->getContents()->getNode(words[1])->getContents()->readfile() << endl;

}

void fn_cd (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   inode_ptr cwd = state.getCwd();
   if (cwd == state.getRoot())
      return;
   state.setCwd(cwd->getContents()->getNode(words[1]));
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
   auto pathList = state.getCwd()->getContents()->getAllPaths();
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
   inode_ptr newFile = state.getCwd()->getContents()->mkfile(words[1]);
   wordvec newData(words.begin()+2, words.end());
   newFile->getContents()->writefile(newData);
}

void fn_mkdir (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if (words.size() <= 0){
      cout << "mkdir: missing operand" << endl;
      return;
   }

   for (auto it = words.begin()+1; it != words.end(); ++it){
      auto newDir = state.getCwd()->getContents()->mkdir(*it);
      newDir->getContents()->setPath("..", state.getCwd());
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
   cout << state.getCwd()->getContents()->getPath(state.getCwd()) << endl;
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

