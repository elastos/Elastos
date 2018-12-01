package payload

const PayloadUpdateProducerVersion byte = 0x00

type PayloadUpdateProducer struct {
	*PayloadRegisterProducer
}
