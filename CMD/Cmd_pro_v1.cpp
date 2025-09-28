#include <sstream>
#include <vector>
#include <unordered_map>
#include <functional>
#include <iostream>
#include <memory>
using namespace std;

enum class RETURN_STATUS
{
    SUCCESS = 0,
    INTERNAL_ERR = 1,
    INVALID_ARGS = 2
};
class CommandRegistry;
class ICommand
{
    public:
    virtual RETURN_STATUS execute(vector<string>& args, int argc) const = 0;
    virtual CommandRegistry& getRegistry() = 0;
    virtual string getDesc() const = 0;
    virtual string getName() const = 0;
    virtual bool hasAction() const {
        return false;
    }
    virtual ~ICommand() = default;
};
class IParser
{
    public:
    virtual vector<string> tokenize(istringstream& ss) = 0;
    virtual ~IParser() =default;
};
class WithoutQuoteParser:public IParser
{
    public:
    vector<string> tokenize(istringstream& ss) override
    {
        vector<string> tokens;
        while(ss)
        {
            string word;
            ss >> word;
            tokens.push_back(word);
        }
        return tokens;
    }
};
using commandTask = function<void(vector<string>&, int )>;

class CommandRegistry
{
    unordered_map<string,unique_ptr<ICommand>> subCommands;
    public:
    CommandRegistry& addSubCommand(unique_ptr<ICommand> cmd)
    {
        if(subCommands.find(cmd->getName()) != subCommands.end())
        {
            cout<<cmd->getName()<<" Already Exist"<<endl;
        }
        else
        {
            subCommands[cmd->getName()] = std::move(cmd);
        }
        return *this;
    }
    ICommand* getSubCommand(string key)
    {
        if(subCommands.find(key) == subCommands.end())
        {
            return nullptr;
        }
        return subCommands[key].get();
    }
    bool hasSubCommand(string key)
    {
        return subCommands.find(key) != subCommands.end();
    }
    string help()
    {
        ostringstream os;
        os << "----------------------------------\n";
        os << "     Please find Usage        \n";
        for(auto&cmd : subCommands)
        {
            os<<cmd.first <<"   -  "<<cmd.second->getDesc()<<endl;
        }
        os << "----------------------------------\n";
        return os.str();
    }

};
class CommandHandler
{
    static const unique_ptr<ICommand> root;
    // unique_ptr<WithoutQuoteParser> p = make_unique<WithoutQuoteParser>();
    unique_ptr<IParser> parser =  make_unique<WithoutQuoteParser>();
    public:
    void execute(string cmd)
    {
        istringstream ss(cmd);
        
        vector<string>  token = parser->tokenize(ss);
        ICommand* toBeExecuted = root.get();
        for(int i=0;i<static_cast<int>(token.size());i++)
        {
            if(toBeExecuted == nullptr)
            {
                cout<<"Invalid Command - " <<cmd<<endl;
                return;
            }
            if(toBeExecuted->getRegistry().hasSubCommand(token[i]))
            {
                toBeExecuted = toBeExecuted->getRegistry().getSubCommand(token[i]);
            }
            else
            {
                cout<<toBeExecuted->getRegistry().help();
                return;
            }
            if(toBeExecuted->hasAction())
            {
                toBeExecuted->execute(token, i);
                return;
            }
          
        }
    }
    CommandRegistry& addSubCommand(unique_ptr<ICommand>newCmd, ICommand* baseCmd)
    {
        return baseCmd->getRegistry().addSubCommand(std::move(newCmd));
    }
    ICommand* getRoot()
    {
        return root.get();
    }
};

class InternalCommand:public ICommand
{
    string key;
    string desc;
    commandTask action;
    CommandRegistry registry;
    public:
    InternalCommand(string key,string desc):key(key),desc(desc){}
    InternalCommand(string key,string desc,commandTask action):key(key),desc(desc),action(action){}
    RETURN_STATUS execute(vector<string>& args, int argc) const override 
    {
        if(action)
        {
            action(args,argc);
            return RETURN_STATUS::SUCCESS;
        }
        return RETURN_STATUS::INVALID_ARGS;
    }
    CommandRegistry& getRegistry() override {return registry;}
    string getName() const override {return key;}
    string getDesc() const override {return desc;}
    bool hasAction() const override
    { 
        if(action){
            return true;
        }
        return false;
    }
};
const unique_ptr<ICommand> CommandHandler::root = make_unique<InternalCommand>("","root");
class Shell
{
    Shell(){};
    public:
        static unique_ptr<CommandHandler> executer;
        static Shell& getInstance()
        {
            static Shell instance;
            return instance;
        }
        void Run()
        {
            while(true)
            {
                string cmd;
                cout<<">"<<flush;
                getline(cin,cmd);
                executer->execute(cmd);
            }
        }
};

unique_ptr<CommandHandler> Shell::executer = make_unique<CommandHandler>();

int main()
{
    Shell& shell = Shell::getInstance();
    // register some commands
    unique_ptr<ICommand> familiesCmd = make_unique<InternalCommand>("families","available families options");
    ICommand* familiesCmdPtr = familiesCmd.get();
    unique_ptr<ICommand> systemCmd = make_unique<InternalCommand>("sys","system options");
    ICommand* sysPtr = systemCmd.get();

    Shell::executer->addSubCommand(std::move(familiesCmd),Shell::executer->getRoot()).addSubCommand(std::move(systemCmd));
    commandTask showAllFamiliesTask = [](vector<string>& argv,int argc){
            cout<<"executing ...."<<endl;
            for(int i=argc;i<static_cast<int>(argv.size());i++)
            {
                cout<<argv[i]<<" "<<flush;
            }
            cout<<endl<<"[done]"<<endl;
    };
    commandTask systemTask = [](vector<string>& argv,int argc){
        cout<<"executing ...."<<endl;
        for(int i=argc;i<static_cast<int>(argv.size());i++)
        {
            cout<<argv[i]<<" "<<flush;
        }
        cout<<endl<<"[done]"<<endl;
    };
    unique_ptr<ICommand> showAllFamilies = make_unique<InternalCommand>("showall","show all",showAllFamiliesTask);
    unique_ptr<ICommand> showAllsystem = make_unique<InternalCommand>("all","all setting",systemTask);
    Shell::executer->addSubCommand(std::move(showAllFamilies), familiesCmdPtr);
    Shell::executer->addSubCommand(std::move(showAllsystem), sysPtr);

    shell.Run();

}