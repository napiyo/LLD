#include<vector>
#include <unordered_map>
#include<iostream>
#include <stdexcept>

using namespace std;
using Location = pair<int,int>;
enum class VehicalType
{
    BIKE,
    CAR,
    TRUCK
};
enum class ParkingSpotType
{
    SMALL,
    MEDIUM,
    LARGE,
    BLOCKED
};
class ParkingSpot
{ 
    const int id;
    const Location location;
    int ticketId;
    const ParkingSpotType type;
    mutex mtx;
    ParkingSpot(int id,Location loc,ParkingSpotType type):id(id),location(loc),ticketId(0),type(type){}
    bool occupy(int ticketId)
    {
        lock_guard<mutex> lock(mtx);
        if(type==ParkingSpotType::BLOCKED)
        {
            cout<<"Spot is BLOCKED, can not be used"<<endl;
            return false;
        }
        if(ticketId !=0 ) 
        {
            cout<<"Spot is already occupied"<<endl;
            return false;
        }
        this->ticketId = ticketId;
        return true;
    }
    bool free()
    {
        lock_guard<mutex> lock(mtx);
        ticketId = 0;
    }
    public:
    friend class Floor;
    bool isFree()
    {
        lock_guard<mutex> lock(mtx);
        return ticketId==0;
    }   

};
class Floor
{
    int id;
    vector<vector<ParkingSpot>> parkingSpots;
    unordered_map<ParkingSpotType,int> availableSpots;
    Floor(int id,int length,int width,unordered_map<ParkingSpotType,int> parkingSpotsInfo):id(id){
        int i=0,j=0;
        parkingSpots.resize(length);
        for(auto _parkingtype: parkingSpotsInfo)
        {
            for(int k=0;k<_parkingtype.second;k++)
            {
                ParkingSpot _parkingSpot(id+i+j,{i,j},_parkingtype.first);
                parkingSpots[i].push_back( _parkingSpot);
                j++;
                availableSpots[_parkingtype.first] = _parkingtype.second;
                if(j == width){
                    j=0;
                    i++;
                }
            }
        }
       
        for(;i<length;i++)
        {
            for(;j<width;j++)
            {
               
                ParkingSpot _parkingSpot(id+i+j,{i,j},ParkingSpotType::BLOCKED);
                parkingSpots[i].push_back(_parkingSpot);
            }
        }
    }
    unordered_map<ParkingSpotType,int>& avaiableSpotsStats()
    {
        return availableSpots;
    }
    public:
    unordered_map<ParkingSpotType,int> getAvaiableSpotsStats()
    {
        return availableSpots;
    }
    friend class ParkingLot;
    friend class ParkingStrategy;
};
class Vehical
{
    string licence;
    VehicalType type;
    public:
    Vehical(string licence , VehicalType type):licence(licence),type(type){}
    string getLicence() {return licence;}
    VehicalType getType() {return type;}
};
class Ticket
{
    int id;
    string licence;
    int floorId, spotId;
    int timestamp;
    Ticket(int id, string licence):id(id),licence(licence),timestamp(0){}
};
class Gate
{
    protected:
    const int id;
    const Location location;
    Gate(int id,Location loc):id(id),location(loc){}
    public:
    Location getLocation(){
        return location;
    }
    // virtual void dummy() = 0;
    virtual ~Gate() = default;
};
class EntryGate: public Gate
{
    EntryGate(int id,Location loc):Gate(id,loc){}
    public:
    friend class ParkingLot;
    bool park(Vehical vehical)
    {

    } 
};
class ExitGate:public Gate
{
    ExitGate(int id,Location loc):Gate(id,loc){}
    public:
    friend class ParkingLot;
    bool exit(Vehical vehical)
    {

    } 

};
class ParkingLot
{
    int id,length,width;
    unordered_map<int,unique_ptr<Ticket>> tickets;
    vector<EntryGate> entryGates;
    vector<ExitGate> exitGates;
    vector<Floor> floors;
   
    ParkingLot(int id):id(id){}
    void addFloor(unordered_map<ParkingSpotType,int> parkingSpotsInfo){
        Floor _floor(floors.size(),this->length,this->width,parkingSpotsInfo);
        floors.push_back(_floor);
    };
    EntryGate& addEntryGate(Location loc)
    {
        EntryGate _entryGate(entryGates.size(),loc);
        entryGates.push_back(_entryGate);
        return _entryGate;
    }
    ExitGate& addExitGate(Location loc)
    {
        ExitGate _exitGate(exitGates.size(),loc);
        exitGates.push_back(_exitGate);
        return _exitGate;
    }
    vector<Floor>& getFloors()
    {
        return floors;
    }
    public:
    EntryGate& getEntryGate(int id=0)
    {
        if(entryGates.size() >= id) 
        {
            throw runtime_error("Invalid Gate Id");
        }
        return entryGates[id];
    }
    EntryGate& getExitGate(int id=0)
    {
        if(exitGates.size() >= id) 
        {
            throw runtime_error("Invalid Gate Id");
        }
        return entryGates[id];
    }
    friend class ParkingLotManager;
    friend class ParkingService;
};
class ParkingLotManager
{
    unordered_map<int,shared_ptr<ParkingLot>> parkingLots;
    ParkingLotManager(){}
    public:
    ParkingLotManager& getInstance(){
        static ParkingLotManager parkingLotMgr;
        return parkingLotMgr;
    }
    shared_ptr<ParkingLot> getParkingLot(int id)
    {
        if(parkingLots.count(id)) return parkingLots[id];
        cout<<"No parkingLot with id : "<<id<<endl;
        return nullptr;
    }
    shared_ptr<ParkingLot> createParkingLot()
    {
        shared_ptr<ParkingLot> parkingLot = make_shared<ParkingLot>(parkingLots.size());
        parkingLots[parkingLots.size()] = parkingLot;
        return parkingLot;
    }
    shared_ptr<ParkingLot> addFloor(int parkingLotId,unordered_map<ParkingSpotType,int> parkingSpotsInfo)
    {
        if(parkingLots.count(parkingLotId))
        {
            parkingLots[parkingLotId]->addFloor(parkingSpotsInfo);
            return parkingLots[parkingLotId];
        }
        else {
            cout<<"invalid Parking Lot id";
            return nullptr;
        }
    }
};
enum class ParkingStrategyTypes
{
    NEAREST,
    FARTHEST
};
class ParkingStrategy
{
    protected:
    int parkingLotId;
    int hasFreeParkingSpot()
    {
        
    }
    public:
    virtual ~ParkingStrategy() = default;
    virtual ParkingSpot findSpot(int parkingLotId,int entryGateId, Vehical& vehical) = 0;
};
class NearestParkingStrategy : public ParkingStrategy
{
    int park;
    public:
    NearestParkingStrategy(int parkingLotId):parkingLotId(parkingLotId)
    ParkingSpot findSpot(int parkingLotId,int entryGateId, Vehical& vehical) override 
    {

    }


};
class ParkingService
{

};
int main()
{
    return 0;
}