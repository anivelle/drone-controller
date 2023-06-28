using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class PIDController : MonoBehaviour {

    [SerializeField]
    float Kp = 3;
    [SerializeField]
    float Ki = 2;
    [SerializeField]
    float Kd = 1;
    [SerializeField]
    float error, output, sum, prevError;
    float curHeight;
    float targetHeight = 10;
    float timeStep;

    [SerializeField]
    float max, min = 0;

    Rigidbody rb;
    [SerializeField]
    Vector3[] corners = new Vector3[4];
    [SerializeField]
    Vector3 force;
    Vector3 scale;

    // Start is called before the first frame update
    void Start() {
        rb = GetComponent<Rigidbody>();
        scale = transform.lossyScale;
        sum = 0;
        prevError = 0;
        error = 0;
    }

    // Update is called once per frame
    void FixedUpdate() {
        float diff = (max - min) / 2;
        timeStep = Time.fixedDeltaTime;
        for (int i = 0; i < 4; i++) {
            corners[i] = this.transform.position;
            corners[i].x += Mathf.Pow(-1, (i & 2) >> 1) * (scale.x / 2 - 0.3f);
            corners[i].y -= scale.y / 2 + 0.1f;
            corners[i].z += Mathf.Pow(-1, i & 1) * (scale.z / 2 - 0.3f);
        }
        curHeight = transform.position.y;
        prevError = error;
        sum += error * timeStep;  // Just a parameter to simulate time difference
        error = targetHeight - curHeight;

        //if (Mathf.Abs(error) > 1) {
        output = Kp * error + Ki * sum + Kd * (error - prevError) / timeStep;

        // Maps the output to [min, max] using arctan and multiplies this with an upwards force
        force = transform.up * (2 * diff * Mathf.Atan(output * timeStep / 10) / Mathf.PI + (max + min) / 2) * Random.Range(0.8f, 1.2f);
        //      }

        for (int i = 0; i < 4; i++) {
            rb.AddForceAtPosition(force, corners[i]);
        }
        if (Input.GetKey(KeyCode.U))
            rb.AddForceAtPosition(force, corners[2]);
        if (Input.GetKey(KeyCode.I))
            rb.AddForceAtPosition(force, corners[0]);
        if (Input.GetKey(KeyCode.J))
            rb.AddForceAtPosition(force, corners[3]);
        if (Input.GetKey(KeyCode.K))
            rb.AddForceAtPosition(force, corners[1]);
    }
}
