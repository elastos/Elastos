# Generated by the gRPC Python protocol compiler plugin. DO NOT EDIT!
import grpc

from . import health_check_pb2 as health__check__pb2


class HealthStub(object):
  # missing associated documentation comment in .proto file
  pass

  def __init__(self, channel):
    """Constructor.

    Args:
      channel: A grpc.Channel.
    """
    self.Check = channel.unary_unary(
        '/health_check.Health/Check',
        request_serializer=health__check__pb2.HealthCheckRequest.SerializeToString,
        response_deserializer=health__check__pb2.HealthCheckResponse.FromString,
        )
    self.Watch = channel.unary_stream(
        '/health_check.Health/Watch',
        request_serializer=health__check__pb2.HealthCheckRequest.SerializeToString,
        response_deserializer=health__check__pb2.HealthCheckResponse.FromString,
        )


class HealthServicer(object):
  # missing associated documentation comment in .proto file
  pass

  def Check(self, request, context):
    # missing associated documentation comment in .proto file
    pass
    context.set_code(grpc.StatusCode.UNIMPLEMENTED)
    context.set_details('Method not implemented!')
    raise NotImplementedError('Method not implemented!')

  def Watch(self, request, context):
    # missing associated documentation comment in .proto file
    pass
    context.set_code(grpc.StatusCode.UNIMPLEMENTED)
    context.set_details('Method not implemented!')
    raise NotImplementedError('Method not implemented!')


def add_HealthServicer_to_server(servicer, server):
  rpc_method_handlers = {
      'Check': grpc.unary_unary_rpc_method_handler(
          servicer.Check,
          request_deserializer=health__check__pb2.HealthCheckRequest.FromString,
          response_serializer=health__check__pb2.HealthCheckResponse.SerializeToString,
      ),
      'Watch': grpc.unary_stream_rpc_method_handler(
          servicer.Watch,
          request_deserializer=health__check__pb2.HealthCheckRequest.FromString,
          response_serializer=health__check__pb2.HealthCheckResponse.SerializeToString,
      ),
  }
  generic_handler = grpc.method_handlers_generic_handler(
      'health_check.Health', rpc_method_handlers)
  server.add_generic_rpc_handlers((generic_handler,))
