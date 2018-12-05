network = dpos_network.new()
ars = arbitrators.new()
manager = dpos_manager.new(network, ars)

if (manager ~= nil)
then
    print(manager:dump_consensus())
else
    print("create manager fail!")
end
