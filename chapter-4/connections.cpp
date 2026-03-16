// Listing 4.10 Handling multiple connections from a single thread using promises
#include <future>

void process_connections(connection_set& connections)
{
 while(!done(connections)) // 1) Loops until done() return true
    {
        // 2) In the loop it checks each connection
    for( connection_iterator connection=connections.begin(),end=connections.end(); connection!=end; ++connection)
    {
        if(connection->has_incoming_data()) // 3) Retries incoming data
        {
            data_packet data=connection->incoming();
            std::promise<payload_type>& p= connection->get_promise(data.id); // 4) Id is mapped to a promise
            p.set_value(data.payload); 
        }

        if(connection->has_outgoing_data()) // 5) Retries outgoing data
        {
            outgoing_packet data= connection->top_of_outgoing_queue(); // 5.1) Packet is rettried from outgoing_data_queue
            connection->send(data.payload); 
            data.promise.set_value(true); // 6) Promise connected with the data is sent to true
        }
    }
 }
}