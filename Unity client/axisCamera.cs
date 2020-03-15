using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class axisCamera : MonoBehaviour {

    private Transform mainCamera;
    // Zoom
    public float zoomSpeed;
    private Vector3 axisVec;
    // Camera distance
    public float Distance;
    // Camera rotation
    public Quaternion targetRotation;   // 최종 축적된 Gap이 저장되는 변수
    public float rotationSpeed;            // 회전 스피드
    public Vector3 Gap;                    // 회전 축적 값
    public Transform moveVec;
    public Transform parentTrans;



	// Use this for initialization
	void Awake () {
        mainCamera = Camera.main.transform;
        parentTrans = transform.parent;
	}
	
	// Update is called once per frame
	void Update () {
        Zoom();
        Rotation();
	}

    void Zoom()
    {
        Distance += Input.GetAxis("Mouse ScrollWheel") * zoomSpeed * -1;
        Distance = Mathf.Clamp(Distance, 1f, 5f);
        axisVec = transform.forward * -1;
        axisVec += transform.up * 0.3f;
        axisVec *= Distance;
        parentTrans.position = parentTrans.position + axisVec;
    } 

    void Rotation()
    {
        if (transform.rotation != targetRotation)
        {
            // transform.rotation = targetRotation;
            parentTrans.rotation = Quaternion.Slerp(transform.rotation, targetRotation, rotationSpeed * Time.deltaTime);
        }
        if (Input.GetMouseButton(1))
        {
            // 값을 축적
            Gap.x += Input.GetAxis("Mouse Y") * rotationSpeed * -1;
            Gap.y += Input.GetAxis("Mouse X") * rotationSpeed;

            // 카메라 회전 범위 제한
            Gap.x = Mathf.Clamp(Gap.x, -25f, 35f);
            // 회전 값을 변수에 저장
            targetRotation = Quaternion.Euler(Gap);
/*
             Quaternion q = targetRotation;
            q.x = q.z = 0;
            moveVec.transform.rotation = q;
            */
        }
    }

    public axisCamera getThis()
    {
        return this;
    }
}
