local block_utils = {}

block_utils.genesis_block_hash = "05f458a5522851622cae2bb138498dec60a8f0b233802c97a1ca41f9f214708d"

function block_utils.height_one()
    b = block.new()
    h = header.new(1, block_utils.genesis_block_hash, 1)
    b:set_header(h)
    b:update()

    return b
end

return block_utils