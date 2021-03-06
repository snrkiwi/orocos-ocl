#include <rtt/os/main.h>
#include <reporting/TcpReporting.hpp>
#include <taskbrowser/TaskBrowser.hpp>

#include <rtt/SlaveActivity.hpp>
#include <rtt/PeriodicActivity.hpp>
#include <rtt/Ports.hpp>

using namespace std;
using namespace Orocos;
using namespace RTT;

class TestTaskContext
    : public TaskContext
{
    Property<string> hello;
    WriteDataPort<std::vector<double> > dwport;
    ReadDataPort<double> drport;
    std::vector<double> init;
    int pos;

    public:
        TestTaskContext(std::string name)
    : TaskContext(name),
        hello("Hello", "The hello thing", "World"),
        dwport("D2Port"),
        drport("D1Port"),
        init(10,1.0)
        {
            this->properties()->addProperty( & hello );
            this->ports()->addPort( &drport );
            this->ports()->addPort( &dwport );
            pos = 10;
            Logger::log() << Logger::Info << "TestTaskContext initialized" << Logger::endl;

        // write initial value.
            dwport.Set( init );
        }

        virtual bool startup () {
            return true;
        }

        virtual void update () {
            if( pos > 9 )
            {
                init[2]+=2;
            } else if( pos > 3 ) {
                init[2]++;
            } else {
                init[2]--;
            }
            if( pos > 5 ) {
              init[4] += 4;
            } else {
              init[4] -= 4;
            }
            pos--;
            if( pos == 0 ) { pos = 10; }
            dwport.Set( init );
        }

        virtual void shutdown () {

        }
};

class TestTaskContext2
    : public TaskContext
{
    Property<string> hello;
    WriteDataPort<double> dwport;
    ReadDataPort<std::vector<double> > drport;

    public:
        TestTaskContext2(std::string name)
    : TaskContext(name),
        hello("Hello", "The hello thing", "World"),
        dwport("D1Port"),
        drport("D2Port")
        {
            this->properties()->addProperty( & hello );
            this->ports()->addPort( &drport );
            this->ports()->addPort( &dwport );
        }
};

int ORO_main( int argc, char** argv)
{
    // Set log level more verbose than default,
    // such that we can see output :
    if ( Logger::log().getLogLevel() < Logger::Info ) {
        Logger::log().setLogLevel( Logger::Info );
        Logger::log() << Logger::Info << argv[0] << " manually raises LogLevel to 'Info' (5). See also file 'orocos.log'."<<Logger::endl;
    }


    PeriodicActivity act(10, 1.0);
    TcpReporting rc("TCPReporting");
    TestTaskContext gtc("MyPeer");
    TestTaskContext2 gtc2("MyPeer2");
    TestTaskContext2 gtc3("MySoloPeer");

    PeriodicActivity act1(10, 2.0);

    rc.addPeer( &gtc );
    rc.addPeer( &gtc2 );
    rc.addPeer( &gtc3 );
    gtc.connectPeers( &gtc2 );

    TaskBrowser tb( &rc );

    act1.run( gtc.engine() );
    act.run( rc.engine() );
    tb.loop();
    act.stop();
    act1.stop();
    return 0;
}

