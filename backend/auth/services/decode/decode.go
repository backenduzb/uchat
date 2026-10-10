package decode

import (
	"encoding/json"
	"io"
	"net/http"
)

func DecodeJSON[T any](w http.ResponseWriter, r *http.Request) (T, bool) {
	var input T

	r.Body = http.MaxBytesReader(w, r.Body, 4096)

	decoder := json.NewDecoder(r.Body)
	decoder.DisallowUnknownFields()

	if err := decoder.Decode(&input); err != nil {
		http.Error(w, `{"error":"invalid JSON body"}`,
			http.StatusBadRequest)
		return input, false
	}

	var extra any
	if err := decoder.Decode(&extra); err != io.EOF {
		http.Error(w, `{"error":"request must contain a single JSON object"}`,
			http.StatusBadRequest)
		return input, false
	}

	return input, true
}