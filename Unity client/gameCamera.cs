using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class gameCamera : MonoBehaviour {

    public float moveSpeed = 5f;
    public float zoomSpeed = 5f;
    public float rotationSpeed = 5f;

    public float offsetX, offsetY, offsetZ;

    public Transform Target;
    private Quaternion targetRotation;
    public Vector3 rotationGap;
    public Transform mainCameraTransform;
    
    private void Awake()
    {
        Gamekit3D.PlayerController playerController = FindObjectOfType<Gamekit3D.PlayerController>();

        if (playerController != null)
        {
            Target = playerController.transform.Find("HeadTarget");

            if (playerController.cameraSettings == null)
                playerController.cameraSettings = this;
        }
    }

    // Use this for initialization
    void Start () {
        mainCameraTransform = Camera.main.transform;
        if(Target)
            transform.position = Target.transform.position;
        Vector3 cameraVector = new Vector3((transform.position.x + offsetX), (transform.position.y + offsetY), (transform.position.z + offsetZ));
        mainCameraTransform.position = cameraVector;
    }
	
	// Update is called once per frame
	void Update () {
        if (Target != null)
        {
            transform.position += (Target.position - transform.position) * moveSpeed;
            Zoom();
            Rotation();
        }
    }

    float Distance;
    void Zoom()
    {
        Distance += Input.GetAxis("Mouse ScrollWheel") * zoomSpeed * -1;
        Distance = Mathf.Clamp(Distance, 1f, 5f);

        Vector3 zoomVector = transform.forward * -1;
        zoomVector += transform.up * 0.3f;
        zoomVector *= Distance;

        transform.position += zoomVector;
    }

    void Rotation()
    {
        if (transform.rotation != targetRotation)
            transform.rotation = Quaternion.Slerp(transform.rotation, targetRotation, rotationSpeed * Time.deltaTime);
        if (Input.GetMouseButton(1))
        {
            // 값을 축적
            rotationGap.x += Input.GetAxis("Mouse Y") * rotationSpeed * -1;
            rotationGap.y += Input.GetAxis("Mouse X") * rotationSpeed;

            // 카메라 회전 범위 제한
            rotationGap.x = Mathf.Clamp(rotationGap.x, -25f, 25f);
            // 회전 값을 변수에 저장
            targetRotation = Quaternion.Euler(rotationGap);

        }
    }

    public void setTarget(Transform _head)
    {
        Target = _head;
    }


}
