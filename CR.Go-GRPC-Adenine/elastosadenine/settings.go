package elastosadenine

import "time"

// Max grpc message to transfer
const grpcMaxMsgSize int = 1024 * 1024 * 1024

// Timeout to use
const requestTimeout time.Duration = 30 * time.Second

// JWT Settings
const tokenExpiration time.Duration = 24 * 30 * time.Hour
