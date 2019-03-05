local block_utils = {}

block_utils.genesis_block_hash = "05f458a5522851622cae2bb138498dec60a8f0b233802c97a1ca41f9f214708d"

function block_utils.new_block(hash, height)
    b = block.new()
    h = header.new(1, hash, height)
    b:set_header(h)
    b:update()

    return b
end

function block_utils.height_one()
    return block_utils.new_block(block_utils.genesis_block_hash, 1)
end

function block_utils.new_block_from_previous(pre_block)
    return block_utils.new_block(pre_block:hash(), pre_block:height() + 1)
end

return block_utils