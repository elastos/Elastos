// Copyright 2017 The Elastos.ELA.SideChain.ETH Authors
// This file is part of the Elastos.ELA.SideChain.ETH library.
//
// The Elastos.ELA.SideChain.ETH library is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// The Elastos.ELA.SideChain.ETH library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with the Elastos.ELA.SideChain.ETH library. If not, see <http://www.gnu.org/licenses/>.

package client

import (
	"bytes"
	"io/ioutil"
	"os"
	"path/filepath"
	"reflect"
	"sort"
	"testing"

	"github.com/elastos/Elastos.ELA.SideChain.ETH/swarm/storage/feed/lookup"

	"github.com/elastos/Elastos.ELA.SideChain.ETH/common"
	"github.com/elastos/Elastos.ELA.SideChain.ETH/crypto"
	"github.com/elastos/Elastos.ELA.SideChain.ETH/swarm/api"
	swarmhttp "github.com/elastos/Elastos.ELA.SideChain.ETH/swarm/api/http"
	"github.com/elastos/Elastos.ELA.SideChain.ETH/swarm/multihash"
	"github.com/elastos/Elastos.ELA.SideChain.ETH/swarm/storage/feed"
	"github.com/elastos/Elastos.ELA.SideChain.ETH/swarm/testutil"
)

func serverFunc(api *api.API) testutil.TestServer {
	return swarmhttp.NewServer(api, "")
}

// TestClientUploadDownloadRaw test uploading and downloading raw data to swarm
func TestClientUploadDownloadRaw(t *testing.T) {
	testClientUploadDownloadRaw(false, t)
}
func TestClientUploadDownloadRawEncrypted(t *testing.T) {
	testClientUploadDownloadRaw(true, t)
}

func testClientUploadDownloadRaw(toEncrypt bool, t *testing.T) {
	srv := testutil.NewTestSwarmServer(t, serverFunc, nil)
	defer srv.Close()

	client := NewClient(srv.URL)

	// upload some raw data
	data := []byte("foo123")
	hash, err := client.UploadRaw(bytes.NewReader(data), int64(len(data)), toEncrypt)
	if err != nil {
		t.Fatal(err)
	}

	// check we can download the same data
	res, isEncrypted, err := client.DownloadRaw(hash)
	if err != nil {
		t.Fatal(err)
	}
	if isEncrypted != toEncrypt {
		t.Fatalf("Expected encyption status %v got %v", toEncrypt, isEncrypted)
	}
	defer res.Close()
	gotData, err := ioutil.ReadAll(res)
	if err != nil {
		t.Fatal(err)
	}
	if !bytes.Equal(gotData, data) {
		t.Fatalf("expected downloaded data to be %q, got %q", data, gotData)
	}
}

// TestClientUploadDownloadFiles test uploading and downloading files to swarm
// manifests
func TestClientUploadDownloadFiles(t *testing.T) {
	testClientUploadDownloadFiles(false, t)
}

func TestClientUploadDownloadFilesEncrypted(t *testing.T) {
	testClientUploadDownloadFiles(true, t)
}

func testClientUploadDownloadFiles(toEncrypt bool, t *testing.T) {
	srv := testutil.NewTestSwarmServer(t, serverFunc, nil)
	defer srv.Close()

	client := NewClient(srv.URL)
	upload := func(manifest, path string, data []byte) string {
		file := &File{
			ReadCloser: ioutil.NopCloser(bytes.NewReader(data)),
			ManifestEntry: api.ManifestEntry{
				Path:        path,
				ContentType: "text/plain",
				Size:        int64(len(data)),
			},
		}
		hash, err := client.Upload(file, manifest, toEncrypt)
		if err != nil {
			t.Fatal(err)
		}
		return hash
	}
	checkDownload := func(manifest, path string, expected []byte) {
		file, err := client.Download(manifest, path)
		if err != nil {
			t.Fatal(err)
		}
		defer file.Close()
		if file.Size != int64(len(expected)) {
			t.Fatalf("expected downloaded file to be %d bytes, got %d", len(expected), file.Size)
		}
		if file.ContentType != "text/plain" {
			t.Fatalf("expected downloaded file to have type %q, got %q", "text/plain", file.ContentType)
		}
		data, err := ioutil.ReadAll(file)
		if err != nil {
			t.Fatal(err)
		}
		if !bytes.Equal(data, expected) {
			t.Fatalf("expected downloaded data to be %q, got %q", expected, data)
		}
	}

	// upload a file to the root of a manifest
	rootData := []byte("some-data")
	rootHash := upload("", "", rootData)

	// check we can download the root file
	checkDownload(rootHash, "", rootData)

	// upload another file to the same manifest
	otherData := []byte("some-other-data")
	newHash := upload(rootHash, "some/other/path", otherData)

	// check we can download both files from the new manifest
	checkDownload(newHash, "", rootData)
	checkDownload(newHash, "some/other/path", otherData)

	// replace the root file with different data
	newHash = upload(newHash, "", otherData)

	// check both files have the other data
	checkDownload(newHash, "", otherData)
	checkDownload(newHash, "some/other/path", otherData)
}

var testDirFiles = []string{
	"file1.txt",
	"file2.txt",
	"dir1/file3.txt",
	"dir1/file4.txt",
	"dir2/file5.txt",
	"dir2/dir3/file6.txt",
	"dir2/dir4/file7.txt",
	"dir2/dir4/file8.txt",
}

func newTestDirectory(t *testing.T) string {
	dir, err := ioutil.TempDir("", "swarm-client-test")
	if err != nil {
		t.Fatal(err)
	}

	for _, file := range testDirFiles {
		path := filepath.Join(dir, file)
		if err := os.MkdirAll(filepath.Dir(path), 0755); err != nil {
			os.RemoveAll(dir)
			t.Fatalf("error creating dir for %s: %s", path, err)
		}
		if err := ioutil.WriteFile(path, []byte(file), 0644); err != nil {
			os.RemoveAll(dir)
			t.Fatalf("error writing file %s: %s", path, err)
		}
	}

	return dir
}

// TestClientUploadDownloadDirectory tests uploading and downloading a
// directory of files to a swarm manifest
func TestClientUploadDownloadDirectory(t *testing.T) {
	srv := testutil.NewTestSwarmServer(t, serverFunc, nil)
	defer srv.Close()

	dir := newTestDirectory(t)
	defer os.RemoveAll(dir)

	// upload the directory
	client := NewClient(srv.URL)
	defaultPath := testDirFiles[0]
	hash, err := client.UploadDirectory(dir, defaultPath, "", false)
	if err != nil {
		t.Fatalf("error uploading directory: %s", err)
	}

	// check we can download the individual files
	checkDownloadFile := func(path string, expected []byte) {
		file, err := client.Download(hash, path)
		if err != nil {
			t.Fatal(err)
		}
		defer file.Close()
		data, err := ioutil.ReadAll(file)
		if err != nil {
			t.Fatal(err)
		}
		if !bytes.Equal(data, expected) {
			t.Fatalf("expected data to be %q, got %q", expected, data)
		}
	}
	for _, file := range testDirFiles {
		checkDownloadFile(file, []byte(file))
	}

	// check we can download the default path
	checkDownloadFile("", []byte(testDirFiles[0]))

	// check we can download the directory
	tmp, err := ioutil.TempDir("", "swarm-client-test")
	if err != nil {
		t.Fatal(err)
	}
	defer os.RemoveAll(tmp)
	if err := client.DownloadDirectory(hash, "", tmp, ""); err != nil {
		t.Fatal(err)
	}
	for _, file := range testDirFiles {
		data, err := ioutil.ReadFile(filepath.Join(tmp, file))
		if err != nil {
			t.Fatal(err)
		}
		if !bytes.Equal(data, []byte(file)) {
			t.Fatalf("expected data to be %q, got %q", file, data)
		}
	}
}

// TestClientFileList tests listing files in a swarm manifest
func TestClientFileList(t *testing.T) {
	testClientFileList(false, t)
}

func TestClientFileListEncrypted(t *testing.T) {
	testClientFileList(true, t)
}

func testClientFileList(toEncrypt bool, t *testing.T) {
	srv := testutil.NewTestSwarmServer(t, serverFunc, nil)
	defer srv.Close()

	dir := newTestDirectory(t)
	defer os.RemoveAll(dir)

	client := NewClient(srv.URL)
	hash, err := client.UploadDirectory(dir, "", "", toEncrypt)
	if err != nil {
		t.Fatalf("error uploading directory: %s", err)
	}

	ls := func(prefix string) []string {
		list, err := client.List(hash, prefix, "")
		if err != nil {
			t.Fatal(err)
		}
		paths := make([]string, 0, len(list.CommonPrefixes)+len(list.Entries))
		paths = append(paths, list.CommonPrefixes...)
		for _, entry := range list.Entries {
			paths = append(paths, entry.Path)
		}
		sort.Strings(paths)
		return paths
	}

	tests := map[string][]string{
		"":                    {"dir1/", "dir2/", "file1.txt", "file2.txt"},
		"file":                {"file1.txt", "file2.txt"},
		"file1":               {"file1.txt"},
		"file2.txt":           {"file2.txt"},
		"file12":              {},
		"dir":                 {"dir1/", "dir2/"},
		"dir1":                {"dir1/"},
		"dir1/":               {"dir1/file3.txt", "dir1/file4.txt"},
		"dir1/file":           {"dir1/file3.txt", "dir1/file4.txt"},
		"dir1/file3.txt":      {"dir1/file3.txt"},
		"dir1/file34":         {},
		"dir2/":               {"dir2/dir3/", "dir2/dir4/", "dir2/file5.txt"},
		"dir2/file":           {"dir2/file5.txt"},
		"dir2/dir":            {"dir2/dir3/", "dir2/dir4/"},
		"dir2/dir3/":          {"dir2/dir3/file6.txt"},
		"dir2/dir4/":          {"dir2/dir4/file7.txt", "dir2/dir4/file8.txt"},
		"dir2/dir4/file":      {"dir2/dir4/file7.txt", "dir2/dir4/file8.txt"},
		"dir2/dir4/file7.txt": {"dir2/dir4/file7.txt"},
		"dir2/dir4/file78":    {},
	}
	for prefix, expected := range tests {
		actual := ls(prefix)
		if !reflect.DeepEqual(actual, expected) {
			t.Fatalf("expected prefix %q to return %v, got %v", prefix, expected, actual)
		}
	}
}

// TestClientMultipartUpload tests uploading files to swarm using a multipart
// upload
func TestClientMultipartUpload(t *testing.T) {
	srv := testutil.NewTestSwarmServer(t, serverFunc, nil)
	defer srv.Close()

	// define an uploader which uploads testDirFiles with some data
	data := []byte("some-data")
	uploader := UploaderFunc(func(upload UploadFn) error {
		for _, name := range testDirFiles {
			file := &File{
				ReadCloser: ioutil.NopCloser(bytes.NewReader(data)),
				ManifestEntry: api.ManifestEntry{
					Path:        name,
					ContentType: "text/plain",
					Size:        int64(len(data)),
				},
			}
			if err := upload(file); err != nil {
				return err
			}
		}
		return nil
	})

	// upload the files as a multipart upload
	client := NewClient(srv.URL)
	hash, err := client.MultipartUpload("", uploader)
	if err != nil {
		t.Fatal(err)
	}

	// check we can download the individual files
	checkDownloadFile := func(path string) {
		file, err := client.Download(hash, path)
		if err != nil {
			t.Fatal(err)
		}
		defer file.Close()
		gotData, err := ioutil.ReadAll(file)
		if err != nil {
			t.Fatal(err)
		}
		if !bytes.Equal(gotData, data) {
			t.Fatalf("expected data to be %q, got %q", data, gotData)
		}
	}
	for _, file := range testDirFiles {
		checkDownloadFile(file)
	}
}

func newTestSigner() (*feed.GenericSigner, error) {
	privKey, err := crypto.HexToECDSA("deadbeefdeadbeefdeadbeefdeadbeefdeadbeefdeadbeefdeadbeefdeadbeef")
	if err != nil {
		return nil, err
	}
	return feed.NewGenericSigner(privKey), nil
}

// test the transparent resolving of multihash feed updates with bzz:// scheme
//
// first upload data, and store the multihash to the resulting manifest in a feed update
// retrieving the update with the multihash should return the manifest pointing directly to the data
// and raw retrieve of that hash should return the data
func TestClientCreateFeedMultihash(t *testing.T) {

	signer, _ := newTestSigner()

	srv := testutil.NewTestSwarmServer(t, serverFunc, nil)
	client := NewClient(srv.URL)
	defer srv.Close()

	// add the data our multihash aliased manifest will point to
	databytes := []byte("bar")

	swarmHash, err := client.UploadRaw(bytes.NewReader(databytes), int64(len(databytes)), false)
	if err != nil {
		t.Fatalf("Error uploading raw test data: %s", err)
	}

	s := common.FromHex(swarmHash)
	mh := multihash.ToMultihash(s)

	// our feed topic
	topic, _ := feed.NewTopic("foo.eth", nil)

	createRequest := feed.NewFirstRequest(topic)

	createRequest.SetData(mh)
	if err := createRequest.Sign(signer); err != nil {
		t.Fatalf("Error signing update: %s", err)
	}

	feedManifestHash, err := client.CreateFeedWithManifest(createRequest)

	if err != nil {
		t.Fatalf("Error creating feed manifest: %s", err)
	}

	correctManifestAddrHex := "bb056a5264c295c2b0f613c8409b9c87ce9d71576ace02458160df4cc894210b"
	if feedManifestHash != correctManifestAddrHex {
		t.Fatalf("Response feed manifest mismatch, expected '%s', got '%s'", correctManifestAddrHex, feedManifestHash)
	}

	// Check we get a not found error when trying to get feed updates with a made-up manifest
	_, err = client.QueryFeed(nil, "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb")
	if err != ErrNoFeedUpdatesFound {
		t.Fatalf("Expected to receive ErrNoFeedUpdatesFound error. Got: %s", err)
	}

	reader, err := client.QueryFeed(nil, correctManifestAddrHex)
	if err != nil {
		t.Fatalf("Error retrieving feed updates: %s", err)
	}
	defer reader.Close()
	gotData, err := ioutil.ReadAll(reader)
	if err != nil {
		t.Fatal(err)
	}
	if !bytes.Equal(mh, gotData) {
		t.Fatalf("Expected: %v, got %v", mh, gotData)
	}

}

// TestClientCreateUpdateFeed will check that feeds can be created and updated via the HTTP client.
func TestClientCreateUpdateFeed(t *testing.T) {

	signer, _ := newTestSigner()

	srv := testutil.NewTestSwarmServer(t, serverFunc, nil)
	client := NewClient(srv.URL)
	defer srv.Close()

	// set raw data for the feed update
	databytes := []byte("En un lugar de La Mancha, de cuyo nombre no quiero acordarme...")

	// our feed topic name
	topic, _ := feed.NewTopic("El Quijote", nil)
	createRequest := feed.NewFirstRequest(topic)

	createRequest.SetData(databytes)
	if err := createRequest.Sign(signer); err != nil {
		t.Fatalf("Error signing update: %s", err)
	}

	feedManifestHash, err := client.CreateFeedWithManifest(createRequest)

	correctManifestAddrHex := "0e9b645ebc3da167b1d56399adc3276f7a08229301b72a03336be0e7d4b71882"
	if feedManifestHash != correctManifestAddrHex {
		t.Fatalf("Response feed manifest mismatch, expected '%s', got '%s'", correctManifestAddrHex, feedManifestHash)
	}

	reader, err := client.QueryFeed(nil, correctManifestAddrHex)
	if err != nil {
		t.Fatalf("Error retrieving feed updates: %s", err)
	}
	defer reader.Close()
	gotData, err := ioutil.ReadAll(reader)
	if err != nil {
		t.Fatal(err)
	}
	if !bytes.Equal(databytes, gotData) {
		t.Fatalf("Expected: %v, got %v", databytes, gotData)
	}

	// define different data
	databytes = []byte("... no ha mucho tiempo que vivía un hidalgo de los de lanza en astillero ...")

	updateRequest, err := client.GetFeedRequest(nil, correctManifestAddrHex)
	if err != nil {
		t.Fatalf("Error retrieving update request template: %s", err)
	}

	updateRequest.SetData(databytes)
	if err := updateRequest.Sign(signer); err != nil {
		t.Fatalf("Error signing update: %s", err)
	}

	if err = client.UpdateFeed(updateRequest); err != nil {
		t.Fatalf("Error updating feed: %s", err)
	}

	reader, err = client.QueryFeed(nil, correctManifestAddrHex)
	if err != nil {
		t.Fatalf("Error retrieving feed updates: %s", err)
	}
	defer reader.Close()
	gotData, err = ioutil.ReadAll(reader)
	if err != nil {
		t.Fatal(err)
	}
	if !bytes.Equal(databytes, gotData) {
		t.Fatalf("Expected: %v, got %v", databytes, gotData)
	}

	// now try retrieving feed updates without a manifest

	fd := &feed.Feed{
		Topic: topic,
		User:  signer.Address(),
	}

	lookupParams := feed.NewQueryLatest(fd, lookup.NoClue)
	reader, err = client.QueryFeed(lookupParams, "")
	if err != nil {
		t.Fatalf("Error retrieving feed updates: %s", err)
	}
	defer reader.Close()
	gotData, err = ioutil.ReadAll(reader)
	if err != nil {
		t.Fatal(err)
	}
	if !bytes.Equal(databytes, gotData) {
		t.Fatalf("Expected: %v, got %v", databytes, gotData)
	}
}
