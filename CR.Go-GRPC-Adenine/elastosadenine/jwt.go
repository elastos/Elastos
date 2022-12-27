package elastosadenine

import "github.com/dgrijalva/jwt-go"

type JWTClaim struct {
	JwtInfo string `json:"jwt_info"`
	jwt.StandardClaims
}

type Response struct {
	Output string
	Status bool
	StatusMessage string
}

type ResponseData struct {
	Output []byte
	Status bool
	StatusMessage string
}
