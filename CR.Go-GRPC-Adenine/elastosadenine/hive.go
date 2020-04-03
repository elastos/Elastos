package elastosadenine

import (
	"context"
	"crypto/tls"
	"encoding/json"
	"fmt"
	"github.com/cyber-republic/go-grpc-adenine/elastosadenine/stubs/hive"
	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials"
	"github.com/dgrijalva/jwt-go"
	"google.golang.org/grpc/metadata"
	"log"
	"time"
	"os"
	"io/ioutil"
	"crypto/aes"
	"crypto/cipher"
	"crypto/md5"
	"crypto/rand"
	"encoding/hex"
	"io"
)

type Hive struct {
	Connection *grpc.ClientConn
}

type InputUploadAndSign struct {
	Network string `json:"network"`
	PrivateKey string `json:"privateKey"`
	FileContent string `json:"file_content"`
}

type InputVerifyAndShow struct {
	Network string `json:"network"`
	Msg string `json:"msg"`
	Pub string `json:"pub"`
	Sig string `json:"sig"`
	Hash string `json:"hash"`
	PrivateKey string `json:"privateKey"`
}

type ResponseData struct {
	Output []byte
	Status bool
	StatusMessage string
}

func NewHive(host string, port int, production bool) *Hive {
	address := fmt.Sprintf("%s:%d", host, port)
	var opts []grpc.DialOption
	if production == false {
		opts = []grpc.DialOption{
			grpc.WithInsecure(),
		}
	} else {
		config := &tls.Config{
			InsecureSkipVerify: true,
		}
		opts = []grpc.DialOption{
			grpc.WithTransportCredentials(credentials.NewTLS(config)),
		}
	}
	conn, err := grpc.Dial(address, opts...)
	if err != nil {
		log.Fatalf("Failed to connect to gRPC server: %v", err)
	}
	return &Hive{Connection: conn}
}

func (h *Hive) Close() {
	h.Connection.Close()
}

func (h *Hive) UploadAndSign(apiKey, did, network, privateKey, filename string) Response {
	var outputData string
	client := hive.NewHiveClient(h.Connection)

	// Open file for reading
    file, err := os.Open(filename)
    if err != nil {
        log.Fatal(err)
    }
    data, err := ioutil.ReadAll(file)
    if err != nil {
        log.Fatal(err)
    }

    //encryption and encoding
    ciphertext := encrypt(data, privateKey)
	encoded := hex.EncodeToString(ciphertext)

	jwtInfo, _ := json.Marshal(InputUploadAndSign{
		Network: network,
		PrivateKey: privateKey,
		FileContent: encoded,
	})

	claims := JWTClaim{
		JwtInfo: string(jwtInfo),
		StandardClaims: jwt.StandardClaims{
			ExpiresAt: time.Now().UTC().Add(tokenExpiration).Unix(),
		},
	}

	jwtToken := jwt.NewWithClaims(jwt.SigningMethodHS256, claims)
	jwtTokenString, err := jwtToken.SignedString([]byte(apiKey))
	if err != nil {
		log.Fatalf("Failed to execute 'UploadAndSign' method: %v", err)
	}
	md := metadata.Pairs("did", did)
	ctx, cancel := context.WithTimeout(context.Background(), requestTimeout)
	defer cancel()
	ctx = metadata.NewOutgoingContext(ctx, md)

	response, err := client.UploadAndSign(ctx, &hive.Request{Input: jwtTokenString})
	if err != nil {
		log.Fatalf("Failed to execute 'UploadAndSign' method: %v", err)
	}
	
	if response.Status == true {
		recvToken, err := jwt.Parse(response.Output, func(recvToken *jwt.Token) (interface{}, error) {
		    if _, ok := recvToken.Method.(*jwt.SigningMethodHMAC); !ok {
		        return nil, fmt.Errorf("unexpected signing method: %v", recvToken.Header["alg"])
		    }
		    return []byte(apiKey), nil
		})

		if recvClaims, ok := recvToken.Claims.(jwt.MapClaims); ok && recvToken.Valid {
		    strMap := recvClaims["jwt_info"].(map[string]interface{})
			result, _ := json.Marshal(strMap["result"].(map[string]interface{}))
			outputData = string(result)
		} else {
			log.Fatalf("Failed to execute 'UploadAndSign' method: %v", err)
		}
	} 
	responseData := Response{outputData, response.Status, response.StatusMessage}
	return responseData
}

func (h *Hive) VerifyAndShow(apiKey, did, network, privateKey, msg, pub, sig, hash string) ResponseData {
	var outputData []byte
	client := hive.NewHiveClient(h.Connection)

	jwtInfo, _ := json.Marshal(InputVerifyAndShow{
		Network: network,
		Msg: msg,
		Pub: pub,
		Sig: sig,
		Hash: hash,
		PrivateKey: privateKey,
	})

	claims := JWTClaim{
		JwtInfo: string(jwtInfo),
		StandardClaims: jwt.StandardClaims{
			ExpiresAt: time.Now().UTC().Add(tokenExpiration).Unix(),
		},
	}

	jwtToken := jwt.NewWithClaims(jwt.SigningMethodHS256, claims)
	jwtTokenString, err := jwtToken.SignedString([]byte(apiKey))
	if err != nil {
		log.Fatalf("Failed to execute 'VerifyAndShow' method: %v", err)
	}
	md := metadata.Pairs("did", did)
	ctx, cancel := context.WithTimeout(context.Background(), requestTimeout)
	defer cancel()
	ctx = metadata.NewOutgoingContext(ctx, md)

	response, err := client.VerifyAndShow(ctx, &hive.Request{Input: jwtTokenString})
	if err != nil {
		log.Fatalf("Failed to execute 'VerifyAndShow' method: %v", err)
	}
	
	if response.Status == true {
		recvToken, err := jwt.Parse(response.Output, func(recvToken *jwt.Token) (interface{}, error) {
		    if _, ok := recvToken.Method.(*jwt.SigningMethodHMAC); !ok {
		        return nil, fmt.Errorf("unexpected signing method: %v", recvToken.Header["alg"])
		    }
		    return []byte(apiKey), nil
		})

		if recvClaims, ok := recvToken.Claims.(jwt.MapClaims); ok && recvToken.Valid {
		    strMap := recvClaims["jwt_info"].(map[string]interface{})
			ciphertext := strMap["file_content"].(string)
			
			//decoding
			decoded, _ := hex.DecodeString(ciphertext)

			//decryption
			plaintext := decrypt(decoded, privateKey)
			outputData = plaintext
		} else {
			log.Fatalf("Failed to execute 'VerifyAndShow' method: %v", err)
		}
	} 
	responseData := ResponseData{outputData, response.Status, response.StatusMessage}
	return responseData
}

func createHash(key string) string {
	hasher := md5.New()
	hasher.Write([]byte(key))
	return hex.EncodeToString(hasher.Sum(nil))
}

func encrypt(data []byte, passphrase string) []byte {
	block, _ := aes.NewCipher([]byte(createHash(passphrase)))
	gcm, err := cipher.NewGCM(block)
	if err != nil {
		panic(err.Error())
	}
	nonce := make([]byte, gcm.NonceSize())
	if _, err = io.ReadFull(rand.Reader, nonce); err != nil {
		panic(err.Error())
	}
	ciphertext := gcm.Seal(nonce, nonce, data, nil)
	return ciphertext
}

func decrypt(data []byte, passphrase string) []byte {
	key := []byte(createHash(passphrase))
	block, err := aes.NewCipher(key)
	if err != nil {
		panic(err.Error())
	}
	gcm, err := cipher.NewGCM(block)
	if err != nil {
		panic(err.Error())
	}
	nonceSize := gcm.NonceSize()
	nonce, ciphertext := data[:nonceSize], data[nonceSize:]
	plaintext, err := gcm.Open(nil, nonce, ciphertext, nil)
	if err != nil {
		panic(err.Error())
	}
	return plaintext
}
