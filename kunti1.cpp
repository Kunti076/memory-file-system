#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

class File
{
public:
  string content;
};

class Directory
{
public:
  map<string, File> files;
  bool isDirectory;
  Directory *parent;

  Directory() : isDirectory(true), parent(nullptr) {}
};

class FileSystem
{
public:
  Directory root;
  Directory *currentDir;

  FileSystem()
  {
    currentDir = &root;
  }

  void mkdir(const string &path)
  {
    // ... existing code ...
    vector<string> tokens = split(path, '/');
    Directory *cur = currentDir;

    for (size_t i = 0; i < tokens.size(); ++i)
    {
      if (tokens[i] == "..")
      {
        if (cur->parent)
        {
          cur = cur->parent;
        }
        else
        {
          cout << "Error: Cannot go above root directory" << endl;
          return;
        }
      }
      else if (tokens[i] == "/" || tokens[i] == "~")
      {
        cur = &root;
      }
      else if (!tokens[i].empty())
      {
        if (cur->files.find(tokens[i]) == cur->files.end())
        {
          cur->files[tokens[i]] = File();
          cur->files[tokens[i]].isDirectory = true;
          if (i < tokens.size() - 1)
          {
            cur = &cur->files[tokens[i]];
            cur->parent = cur->parent;
          }
        }
        else
        {
          cout << "Error: Directory already exists" << endl;
          return;
        }
      }
    }
  }

  void cd(const string &path)
  {
    // ... existing code ...
    vector<string> tokens = split(path, '/');

    for (const string &token : tokens)
    {
      if (token == "..")
      {
        if (currentDir->parent)
        {
          currentDir = currentDir->parent;
        }
        else
        {
          cout << "Error: Cannot go above root directory" << endl;
          return;
        }
      }
      else if (token == "/" || token == "~")
      {
        currentDir = &root;
      }
      else if (!token.empty())
      {
        if (currentDir->files.find(token) != currentDir->files.end())
        {
          if (currentDir->files[token].isDirectory)
          {
            currentDir = &currentDir->files[token];
          }
          else
          {
            cout << "Error: Cannot cd into a file" << endl;
            return;
          }
        }
        else
        {
          cout << "Error: Directory does not exist" << endl;
          return;
        }
      }
    }
  }

  void ls(const string &path)
  {
    // ... existing code ...
    Directory *dir = currentDir;

    if (!path.empty())
    {
      dir = findDirectory(path);
      if (!dir)
      {
        cout << "Error: Directory does not exist" << endl;
        return;
      }
    }

    for (const auto &file : dir->files)
    {
      if (file.second.isDirectory)
      {
        cout << "<DIR> " << file.first << endl;
      }
      else
      {
        cout << file.first << endl;
      }
    }
  }

  void grep(const string &pattern, const string &path)
  {
    // ... existing code ...
    File file = readFile(path);

    if (file.content.empty())
    {
      cout << "Error: File does not exist or is empty" << endl;
      return;
    }

    if (file.content.find(pattern) != string::npos)
    {
      cout << "Found pattern '" << pattern << "' in file '" << path << "'" << endl;
    }
    else
    {
      cout << "Pattern '" << pattern << "' not found in file '" << path << "'" << endl;
    }
  }

  void cat(const string &path)
  {
    // ... existing code ...
    File file = readFile(path);

    if (file.content.empty())
    {
      cout << "Error: File does not exist or is empty" << endl;
      return;
    }

    cout << file.content << endl;
  }

  void touch(const string &path)
  {
    // ... existing code ...
    vector<string> tokens =
        split(path, '/');
    Directory *cur = currentDir;

    for (size_t i = 0; i < tokens.size() - 1; ++i)
    {
      if (cur->files.find(tokens[i]) == cur->files.end())
      {
        cur->files[tokens[i]] = File();
        cur->files[tokens[i]].isDirectory = true;
        if (i < tokens.size() - 2)
        {
          cur = &cur->files[tokens[i]];
          cur->parent = cur->parent;
        }
      }
      else if (!cur->files[tokens[i]].isDirectory)
      {
        cout << "Error: Cannot create file within a file" << endl;
        return;
      }
    }

    cur->files[tokens[tokens.size() - 1]] = File();
  }

  void echo(const string &content, const string &path)
  {
    // ... existing code ...
    File file = readFile(path);

    if (file.isDirectory)
    {
      cout << "Error: Cannot write to a directory" << endl;
      return;
    }

    file.content += content;
  }

  void mv(const string &source, const string &destination)
  {
    // ... existing code ...
    vector<string> sourceTokens = split(source, '/');
    vector<string> destinationTokens = split(destination, '/');

    // Find source and destination directories
    Directory *sourceDir = findDirectory(sourceTokens);
    Directory *destinationDir = findDirectory(destinationTokens);

    if (!sourceDir || !destinationDir)
    {
      cout << "Error: Source or destination directory does not exist" << endl;
      return;
    }

    // Check if source and destination files exist
    if (sourceDir->files.find(sourceTokens.back()) == sourceDir->files.end())
    {
      cout << "Error: Source file does not exist" << endl;
      return;
    }

    if (destinationDir->files.find(destinationTokens.back()) != destinationDir->files.end())
    {
      cout << "Error: Destination file already exists" << endl;
      return;
    }

    // Move the file
    File file = sourceDir->files[sourceTokens.back()];
    sourceDir->files.erase(sourceTokens.back());
    destinationDir->files[destinationTokens.back()] = file;

    // Update parent pointers
    file.parent = destinationDir;

    cout << "Moved file '" << source << "' to '" << destination << "'" << endl;
  }

  void cp(const string &source, const string &destination)
  {
    // ... existing code ...
    vector<string> sourceTokens = split(source, '/');
    vector<string> destinationTokens = split(destination, '/');

    // Find source and destination directories
    Directory *sourceDir = findDirectory(sourceTokens);
    Directory *destinationDir = findDirectory(destinationTokens);

    if (!sourceDir || !destinationDir)
    {
      cout << "Error: Source or destination directory does not exist" << endl;
      return;
    }

    // Check if source file exists
    if (sourceDir->files.find(sourceTokens.back()) == sourceDir->files.end())
    {
      cout << "Error: Source file does not exist" << endl;
      return;
    }

    // Create a copy of the file and add it to the destination directory
    File file = sourceDir->files[sourceTokens.back()];
    destinationDir->files[destinationTokens.back()] = file;

    // Update parent pointer of the copy
    destinationDir->files[destinationTokens.back()].parent = destinationDir;

    cout << "Copied file '" << source << "' to '" << destination << "'" << endl;
  }

  void rm(const string &path)
  {
    // ... existing code ...
    vector<string> tokens = split(path, '/');
    Directory *dir = findDirectory(tokens);

    if (!dir)
    {
      cout << "Error: Directory does not exist" << endl;
      return;
    }

    if (dir->files.find(tokens.back()) == dir->files.end())
    {
      cout << "Error: File does not exist" << endl;
      return;
    }

    if (dir->files[tokens.back()].isDirectory)
    {
      cout << "Error: Cannot remove directories" << endl;
      return;
    }

    dir->files.erase(tokens.back());

    cout << "Removed file '" << path << "'" << endl;
  }

  void saveState(string path)
  {
    json data;

    // Save root directory structure
    data = to_json(root);

    // Save current working directory
    data["currentDir"] = currentDirPath();

    // Write data to file
    ofstream file(path);
    file << data << endl;
    file.close();

    cout << "State saved to '" << path << "'" << endl;
  }

  void loadState(string path)
  {
    ifstream file(path);
    json data;
    file >> data;
    file.close();

    // Load root directory structure
    root = data.get<Directory>();

    // Set current working directory
    setCurrentDir(data["currentDir"].get<string>());

    cout << "State loaded from '" << path << "'" << endl;
  }

private:
  string currentDirPath()
  {
    string path = "";
    Directory *dir = currentDir;
    while (dir)
    {
      if (!dir->files.empty())
      {
        for (auto it = dir->files.rbegin(); it != dir->files.rend(); ++it)
        {
          if (it->second.isDirectory && it->second.files[it->first] == *currentDir)
          {
            path = "/" + it->first + path;
            break;
          }
        }
      }
      dir = dir->parent;
    }
    return path.empty() ? "/" : path;
  }

  void setCurrentDir(string path)
  {
    vector<string> tokens = split(path, '/');
    currentDir = &root;

    for (const string &token : tokens)
    {
      if (token == "..")
      {
        if (currentDir->parent)
        {
          currentDir = currentDir->parent;
        }
        else
        {
          cout << "Error: Cannot go above root directory" << endl;
          return;
        }
      }
      else if (token == "/" || token == "~")
      {
        currentDir = &root;
      }
      else if (!token.empty())
      {
        if (currentDir->files.find(token) != currentDir->files.end() && currentDir->files[token].isDirectory)
        {
          currentDir = &currentDir->files[token];
        }
        else
        {
          cout << "Error: Directory does not exist" << endl;
          return;
        }
      }
    }
  }

  vector<string> split(const string &str, char delimiter)
  {
    vector<string> tokens;
    string token;
    istringstream stream(str);

    while (getline(stream, token, delimiter))
    {
      if (!token.empty())
      {
        tokens.push_back(token);
      }
    }

    return tokens;
  }
};

int main(int argc, char *argv[])
{
  bool saveState = false;
  bool loadState = false;
  string stateFile;

  for (int i = 1; i < argc; ++i)
  {
    if (strcmp(argv[i], "--save-state") == 0)
    {
      saveState = true;
    }
    else if (strcmp(argv[i], "--load-state") == 0)
    {
      loadState = true;
    }
    else if (strcmp(argv[i], "--state-file") == 0)
    {
      stateFile = argv[++i];
    }
  }

  FileSystem fs;

  if (loadState)
  {
    fs.loadState(stateFile);
  }

  // ... existing code for command loop ...
  string command;

  while (true)
  {
    cout << ">> ";
    getline(cin, command);

    if (command.empty())
    {
      continue;
    }

    if (command == "exit")
    {
      break;
    }

    vector<string> tokens = split(command, ' ');
    string cmd = tokens[0];
    string args = tokens.size() > 1 ? tokens[1] : "";

    if (cmd == "mkdir")
    {
      fs.mkdir(args);
    }
    else if (cmd == "cd")
    {
      fs.cd(args);
    }
    else if (cmd == "ls")
    {
      fs.ls(args);
    }
    else if (cmd == "grep")
    {
      if (tokens.size() < 3)
      {
        cout << "Error: Usage: grep <pattern> <path>" << endl;
      }
      else
      {
        fs.grep(tokens[1], tokens[2]);
      }
    }
    else if (cmd == "cat")
    {
      fs.cat(args);
    }
    else if (cmd == "touch")
    {
      fs.touch(args);
    }
    else if (cmd == "echo")
    {
      if (tokens.size() < 3)
      {
        cout << "Error: Usage: echo <content> <path>" << endl;
      }
      else
      {
        string content = "";
        for (size_t i = 2; i < tokens.size(); ++i)
        {
          content += tokens[i] + " ";
        }
        fs.echo(content, args);
      }
    }
    else if (cmd == "mv")
    {
      if (tokens.size() < 3)
      {
        cout << "Error: Usage: mv <source> <destination>" << endl;
      }
      else
      {
        fs.mv(tokens[1], tokens[2]);
      }
    }
    else if (cmd == "cp")
    {
      if (tokens.size() < 3)
      {
        cout << "Error: Usage: cp <source> <destination>" << endl;
      }
      else
      {
        fs.cp(tokens[1], tokens[2]);
      }
    }
    else if (cmd == "rm")
    {
      fs.rm(args);
    }
    else
    {
      cout << "Error: Unknown command '" << cmd << "'" << endl;
    }
  }

  if (saveState)
  {
    fs.saveState(stateFile);
  }

  return 0;
}