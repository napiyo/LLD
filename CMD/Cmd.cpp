#include <unordered_map>
#include <string>
#include <functional>
#include <iostream>


using namespace std;
/*
*/
class Command
{
    string name;
    string desc;
    unordered_map<string,Command*> commands;
    function<void()> func;
    public:
    Command(string name,string desc,function<void()> fun):name(name),desc(desc),func(fun){}
    ~Command(){
        for(auto cmd:commands)
        {
            if(cmd.second)
            {
                delete cmd.second;
            }
        }
    }
    Command* addCommand(string name,string desc, function<void()> fuc)
    {
        if(hasSubCommands(name)) {
            cout<<"command already exist"<<endl;
            return nullptr;
        }
        
        Command* cmd = new Command(name,desc,fuc);
        commands[name] = cmd;
        return cmd;
        
    }
    string getName(){return name;}
    string getDesc() {return desc;}
    vector<pair<string,string>> getCommands(){
        vector<pair<string,string>> cmds;
        for(auto cmd: commands)
        {   
            cmds.emplace_back(cmd.first,cmd.second->getDesc());
            
        }
        return cmds;
    }
    bool hasSubCommands(string cmd)
    {
        return commands.find(cmd) != commands.end();
    }
    Command* getSubCommand(string cmd)
    {
        if(hasSubCommands(cmd))
        {
            return commands[cmd];
        }
        return nullptr;
    }
    void excute( vector<string> &args, int i)
    {
        if(func)
        {
            // not supporting args, as I dont know how to pass them
            // args from i to last will be arguments of func
            func();
            cout<<name<<" exited successfully"<<endl;
            return;
        }
        if(i>=args.size() || i<0) 
        {
             cout<<name<<"got invalid args"<<endl;
        }
        else if(hasSubCommands(args[i]))
        {
            commands[args[i]]->excute(args,i+1);
        }
        else
        {
            cout<<"invalid args - this command need below arguments -  "<<endl; 
            cout<<"-------------------------------------------------"<<endl;
            for(auto cmd: commands)
            {   
                    cout<<cmd.first<<" - "<<cmd.second->getDesc()<<endl;
            }
            cout<<"-------------------------------------------------"<<endl;
        }
    }
};

class History
{
    vector<string> cmdHistory;
    int userIsAt;
    public:
    History():userIsAt(-1){}
    void addHistory(string line){
        cmdHistory.push_back(line);
        cout<<"History "<<line<<" added"<<endl;
        userIsAt = cmdHistory.size();
    }
    string getLast()
    {
        if(cmdHistory.size()==0) return "";
        return *prev(cmdHistory.end());
    }
    string getPrev()
    {
        if(userIsAt < 1 || userIsAt > cmdHistory.size()) userIsAt = cmdHistory.size();
        if(userIsAt == 0) return "";
        userIsAt--;
        return cmdHistory[userIsAt];
        
    }
    string getNext()
    {
        if(userIsAt < -1 || userIsAt >= cmdHistory.size()) userIsAt = cmdHistory.size()-1;
        if(userIsAt == cmdHistory.size()-1) return "";
        userIsAt++;
        return cmdHistory[userIsAt];
    }
};
class Parser
{
    public:
    vector<string> getToken(string line)
    {
        string curr;
        vector<string> ans;
        for(int i=0;i<line.size();i++)
        {
            char c = line[i];
            if(c==' ') {
                ans.push_back(curr);
                curr = "";
                
            }
            else
            {
                curr.push_back(c);
            }
        }
        if(curr != "") ans.push_back(curr);
        return ans;
    }
    
};

class Shell
{
    History history;
    Parser parser;
    Command* rootCommands;
    public:
    Shell()
    {
        vector<string> dummy;
        rootCommands = new Command("","",nullptr);
        rootCommands->addCommand("help","show all supported commands",[&](){this->rootCommands->excute(dummy,0);});
	    Command* showCommand = rootCommands->addCommand("show","show families",nullptr);
	    showCommand->addCommand("all","show all families",[](){cout<<"showing all families"<<endl;});
	    showCommand->addCommand("vip","show vip families",[](){cout<<" this is VIP"<<endl;});
    }
     void excute(string line)
    {
        history.addHistory(line);
        
       vector<string> tokens =  parser.getToken(line);
       if(tokens.size()==0) return;
       if(rootCommands->hasSubCommands(tokens[0]))
       {
           rootCommands->excute(tokens,0);
       }
       else
       {
           cout<<"> command "<<tokens[0]<<" not found!!"<<endl;
       }
       
    }
    Command* getRootCommand(){return rootCommands;}
    
    History& getHistory(){return history;}
};

int main() {
	Shell shell;
	History& history = shell.getHistory();
	cout<<" history - "<<history.getLast();
	vector<string> cmds = {"show all","show","show vip","help"};
	for(auto cmd:cmds)
	{
	    shell.excute(cmd);
	}
	
	
	

}
