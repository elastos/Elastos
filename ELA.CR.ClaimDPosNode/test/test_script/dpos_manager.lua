network = dpos_network.new()
manager = dpos_manager.new(network)

if (manager ~= nil)
then
    print(manager:dump_consensus())
else
    print("create manager fail!")
end
